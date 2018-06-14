/*
 * ReadMySql.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "ReadMySql.h"

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQMCHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <TRandom3.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

//#define LogInfo(exp)		std::cout<<"INFO: "   <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogDebug(exp)		std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

ReadMySql::ReadMySql(const std::string& name) :
SubsysReco(name),
_hit_type("SQHit_v1"),
_hit_container_type("Vector"),
_server_name("seaquestdb01.fnal.gov"),
_port(3310),
_user_name("seaguest"),
_password("qqbar2mu+mu-"),
_input_shcema("run_014075_R007"),
_table_name("kHit"),
_run_id(-1),
_spill_id(-1),
_input_server(nullptr),
_res(nullptr),
_row(nullptr),
p_geomSvc(nullptr),
_event(0),
_run_header(nullptr),
_spill_map(nullptr),
_event_header(nullptr),
_hit_map(nullptr),
_hit_vector(nullptr)
{
	_event_ids.clear();
}

int ReadMySql::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::InitRun(PHCompositeNode* topNode) {

    char serverUrl[200];
    sprintf(serverUrl, "mysql://%s:%d", _server_name.c_str(), _port);
    _input_server = TSQLServer::Connect(serverUrl, _user_name.c_str(), _password.c_str());
    if(!_input_server) {
    	if(Verbosity() >= Fun4AllBase::VERBOSITY_QUIET)
    		LogError("!_input_server");
    	return Fun4AllReturnCodes::ABORTRUN;
    }

    //check the essential info
    sprintf(_query, "USE %s", _input_shcema.c_str());
    if(!_input_server->Exec(_query))
    {
        std::cout << "MySQLSvc: working schema does not exist! Will exit..." << std::endl;
        return Fun4AllReturnCodes::ABORTRUN;
    }

    int ret = MakeNodes(topNode);
    if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

    sprintf(_query, "SELECT runID FROM Run LIMIT 1");
    makeQueryInput();
    nextEntry();
    int run_id = getInt(_row,0);

  	try {
  		ReadMySql::FillSQRun(_run_header, _input_server, run_id, "Run");
  	} catch (...) {
  		LogWarning("");
  	}

  	try {
  		ReadMySql::FillSQSpill(_spill_map, _input_server, run_id, "Spill");
  	} catch (...) {
  		LogWarning("");
  	}

    sprintf(_query, "SELECT eventID FROM Event");
    int nrow = makeQueryInput();
    _event_ids.clear();
    for(int i = 0; i < nrow; ++i)
    {
        nextEntry();
        _event_ids.push_back(getInt(_row, 0));
    }

    p_geomSvc = GeomSvc::instance();

	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering ReadMySql::process_event: " << _event << std::endl;

	if(_event >= _event_ids.size()) return Fun4AllReturnCodes::ABORTRUN;

	int eventID = _event_ids[_event++];

	try {
		ReadMySql::FillSQEvent(_event_header, _input_server, eventID, "Event");
	} catch (...) {
		LogWarning("");
	}

	_event_header->identify();

	try {
		if(_hit_container_type.find("Map") != std::string::npos) {
			ReadMySql::FillSQHitMap(_hit_map, _input_server, eventID, "Hit");
		}
		if(_hit_container_type.find("Vector") != std::string::npos) {
			ReadMySql::FillSQHitVector(_hit_vector, _hit_type, _input_server, eventID, "Hit");
			ReadMySql::FillSQTriggerHitVector(_triggerhit_vector, _hit_type, _input_server, eventID, "TriggerHit");
		}
	} catch (...) {
		LogWarning("");
	}

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Leaving ReadMySql::process_event: " << _event-1 << std::endl;

	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::End(PHCompositeNode* topNode) {
	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "ReadMySql::End" << std::endl;
	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::MakeNodes(PHCompositeNode* topNode) {
	PHNodeIterator iter(topNode);

	PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
	if (!runNode) {
		LogInfo("No RUN node, create one");
		runNode = new PHCompositeNode("RUN");
		topNode->addNode(runNode);
	}

	_run_header = new SQRun_v1();
	PHIODataNode<PHObject>* runHeaderNode = new PHIODataNode<PHObject>(_run_header,"SQRun", "PHObject");
	runNode->addNode(runHeaderNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SQRun Added");

	_spill_map = new SQSpillMap_v1();
	PHIODataNode<PHObject>* spillNode = new PHIODataNode<PHObject>(_spill_map,"SQSpillMap", "PHObject");
	runNode->addNode(spillNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SQSpillMap Added");

	PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
	if (!eventNode) {
		LogInfo("No DST node, create one");
		eventNode = new PHCompositeNode("DST");
		topNode->addNode(eventNode);
	}

	_event_header = new SQEvent_v1();
	PHIODataNode<PHObject>* eventHeaderNode = new PHIODataNode<PHObject>(_event_header,"SQEvent", "PHObject");
	eventNode->addNode(eventHeaderNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SQEvent Added");

	if(_hit_container_type.find("Map") != std::string::npos) {
		_hit_map = new SQHitMap_v1();
		PHIODataNode<PHObject>* hitNodeMap = new PHIODataNode<PHObject>(_hit_map,"SQHitMap", "PHObject");
		eventNode->addNode(hitNodeMap);
		if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
			LogInfo("DST/SQHitMap Added");
	}

	if(_hit_container_type.find("Vector") != std::string::npos) {
		_hit_vector = new SQHitVector_v1();
		PHIODataNode<PHObject>* hitNodeVector = new PHIODataNode<PHObject>(_hit_vector,"SQHitVector", "PHObject");
		eventNode->addNode(hitNodeVector);
		if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
			LogInfo("DST/SQHitVector Added");

		_triggerhit_vector = new SQHitVector_v1();
		PHIODataNode<PHObject>* triggerhitNodeVector = new PHIODataNode<PHObject>(_triggerhit_vector,"SQTriggerHitVector", "PHObject");
		eventNode->addNode(triggerhitNodeVector);
		if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
			LogInfo("DST/SQTriggerHitVector Added");
	}

	return Fun4AllReturnCodes::EVENT_OK;
}


int ReadMySql::GetNodes(PHCompositeNode* topNode) {

	_run_header = findNode::getClass<SQRun>(topNode, "SQRun");
	if (!_run_header) {
		LogError("!_run_header");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	_spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
	if (!_spill_map) {
		LogError("!_spill_map");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	_event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
	if (!_event_header) {
		LogError("!_event_header");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	if(_hit_container_type.find("Map") != std::string::npos) {
		_hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
		if (!_hit_map) {
			LogError("!_hit_map");
			return Fun4AllReturnCodes::ABORTEVENT;
		}
	}

	if(_hit_container_type.find("Vector") != std::string::npos) {
		_hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
		if (!_hit_vector) {
			LogError("!_hit_vector");
			return Fun4AllReturnCodes::ABORTEVENT;
		}
	}

	return Fun4AllReturnCodes::EVENT_OK;
}



int ReadMySql::makeQueryInput() {

    if(_input_server == NULL) return 0;

    if(_res != NULL) delete _res;
    _res = _input_server->Query(_query);

    if(_res != NULL) return _res->GetRowCount();
    return 0;
}

bool ReadMySql::nextEntry() {
    if(_res == NULL) return false;

    if(_row != NULL) delete _row;
    _row = _res->Next();

    if(_row != NULL) return true;
    return false;
}


int ReadMySql::getInt(TSQLRow* row, int id, int default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    try
    {
        return boost::lexical_cast<int>(row->GetField(id));
    }
    catch(boost::bad_lexical_cast&)
    {
        return default_val;
    }
}

float ReadMySql::getFloat(TSQLRow* row, int id, float default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<float>(row->GetField(id));
}

double ReadMySql::getDouble(TSQLRow* row, int id, double default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<double>(row->GetField(id));
}

std::string ReadMySql::getString(TSQLRow* row, int id, std::string default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return std::string(row->GetField(id));
}

int ReadMySql::FillSQHitMap(SQHitMap* hit_map, TSQLServer* server,
		const int event_id, const char* table) {

	if(!hit_map) {
		LogError("!hitmap");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	//                     0      1             2          3        4
	sprintf(query, "SELECT hitID, detectorName, elementID, tdcTime, inTime, "
			//5       6          7              8           9
			" masked, driftTime, driftDistance, resolution, dataQuality"
			" FROM %s WHERE eventID=%d", table, event_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

    for(int irow = 0; irow < nrow; ++irow)
    {
    	TSQLRow* row = res->Next();

        SQHit *hit = new SQHit_v1();

        hit->set_hit_id(getInt(row,0));

        hit->set_detector_id(0);
        hit->set_element_id(getInt(row,2));

        hit->set_tdc_time(getFloat(row,3));
        hit->set_drift_distance(getFloat(row,7));
        hit->set_pos(0);

        hit->set_in_time(getInt(row,4));
        hit->set_hodo_mask(getInt(row,5));

        hit_map->insert(hit);
    }

	delete res;

	return 0;
}


int ReadMySql::FillSQHitVector(
		SQHitVector* hit_vector,
		const std::string hit_type,
		TSQLServer* server,
		const int event_id,
		const char* table) {


	if(!hit_vector) {
		LogError("!hit_vector");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	//                     0      1             2          3        4
	sprintf(query, "SELECT hitID, detectorName, elementID, tdcTime, inTime, "
			//5       6          7              8           9
			" masked, driftTime, driftDistance, resolution, dataQuality"
			" FROM %s WHERE eventID=%d", table, event_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

	TRandom3 rand;

	for(int irow = 0; irow < nrow; ++irow)
	{
		TSQLRow* row = res->Next();

			SQHit *hit = nullptr;

			if(hit_type.find("SQHit_v1")!=std::string::npos) {
				hit = new SQHit_v1();
			}
			else if (hit_type.find("SQMCHit_v1")!=std::string::npos) {
				hit = new SQMCHit_v1();
				hit->set_track_id(getInt(row,0));
			}

			hit->set_hit_id(getInt(row,0));

			//hit->set_detector_id(static_cast<short>(1000*rand.Rndm()));
			//hit->set_element_id(getInt(row,2));

			std::string detectorName(row->GetField(1));
			int elementID = getInt(row,2);
//			std::cout
//			<< "DEBUG: " << __LINE__
//			<< " detectorName: " << detectorName.c_str()
//			<< " elementID: " << elementID
//			<<std::endl;
			p_geomSvc->toLocalDetectorName(detectorName, elementID);

			hit->set_detector_id(p_geomSvc->getDetectorID(detectorName));
			hit->set_element_id(elementID);

//			std::cout
//			<< "DEBUG: " << __LINE__
//			<< " detectorName: " << detectorName.c_str()
//			<< " elementID: " << elementID
//			<< " detectorID: " << hit->get_detector_id()
//			<<std::endl;

			hit->set_tdc_time(getFloat(row,3));
			hit->set_drift_distance(getFloat(row,7));

			if(getInt(row,4)>0) hit->set_in_time(true);
			if(getInt(row,5)>0) hit->set_hodo_mask(true);

			hit->set_trigger_mask(rand.Rndm()>0.5);

			hit->set_pos(p_geomSvc->getMeasurement(hit->get_detector_id(), hit->get_element_id()));

//			std::cout
//			<< "DEBUG: " << __LINE__
//			<< " pos: " << hit->get_pos()
//			<<std::endl;

			hit_vector->push_back(hit);
	}

	delete res;

	return 0;
}

int ReadMySql::FillSQTriggerHitVector(
		SQHitVector* hit_vector,
		const std::string hit_type,
		TSQLServer* server,
		const int event_id,
		const char* table) {


	if(!hit_vector) {
		LogError("!hit_vector");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	//                     0      1             2          3        4
	sprintf(query, "SELECT hitID, detectorName, elementID, tdcTime, inTime"
			" FROM %s WHERE eventID=%d", table, event_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

	TRandom3 rand;

	for(int irow = 0; irow < nrow; ++irow)
	{
		TSQLRow* row = res->Next();

			SQHit *hit = nullptr;

			if(hit_type.find("SQHit_v1")!=std::string::npos) {
				hit = new SQHit_v1();
			}
			else if (hit_type.find("SQMCHit_v1")!=std::string::npos) {
				hit = new SQMCHit_v1();
				hit->set_track_id(getInt(row,0));
			}

			hit->set_hit_id(getInt(row,0));

			int elementID = getInt(row,2);
			hit->set_element_id(elementID);


			std::string detectorName(row->GetField(1));
      if(detectorName.find("H4T") != std::string::npos || detectorName.find("H4B") != std::string::npos)
      {
          detectorName.replace(3, detectorName.length(), "");
      }
			hit->set_detector_id(p_geomSvc->getDetectorID(detectorName));

			hit->set_tdc_time(getFloat(row,3));
			hit->set_drift_distance(0);

			if(getInt(row,4)>0) hit->set_in_time(true);

			hit->set_pos(p_geomSvc->getMeasurement(hit->get_detector_id(), hit->get_element_id()));

			hit_vector->push_back(hit);
	}

	delete res;

	return 0;
}

int ReadMySql::FillSQEvent(SQEvent* event_header, TSQLServer* server,
		const int event_id, const char* table) {


	if(!event_header) {
		LogError("!event_header");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	sprintf(query,
			//      0      1        2        3
			"SELECT runID, spillID, eventID, codaEventID, "
			//4     5     6     7     8
			" NIM1, NIM2, NIM3, NIM4, NIM5, "
			//9        10       11       12       13
			" MATRIX1, MATRIX2, MATRIX3, MATRIX4, MATRIX5"

			" FROM %s WHERE eventID=%d", table, event_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

    for(int irow = 0; irow < nrow; ++irow)
    {
    	TSQLRow* row = res->Next();

    	event_header->set_run_id(getInt(row, 0));

    	event_header->set_spill_id(getInt(row, 1));

    	event_header->set_event_id(getInt(row, 2));

    	event_header->set_coda_event_id(getInt(row, 3));

    	event_header->set_trigger(SQEvent::NIM1, getInt(row, 4));
    	event_header->set_trigger(SQEvent::NIM2, getInt(row, 5));
    	event_header->set_trigger(SQEvent::NIM3, getInt(row, 6));
    	event_header->set_trigger(SQEvent::NIM4, getInt(row, 7));
    	event_header->set_trigger(SQEvent::NIM5, getInt(row, 8));

    	event_header->set_trigger(SQEvent::MATRIX1, getInt(row, 9));
    	event_header->set_trigger(SQEvent::MATRIX2, getInt(row, 10));
    	event_header->set_trigger(SQEvent::MATRIX3, getInt(row, 11));
    	event_header->set_trigger(SQEvent::MATRIX4, getInt(row, 12));
    	event_header->set_trigger(SQEvent::MATRIX5, getInt(row, 13));
    }

	delete res;

	return 0;
}


int ReadMySql::FillSQRun(SQRun* run_header, TSQLServer* server,
		const int run_id, const char* table) {


	if(!run_header) {
		LogError("!run_header");
		return -1;
	}

	run_header->set_run_id(run_id);

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	sprintf(query,
			//      0      1        2
			"SELECT runID, name, value"
			" FROM %s WHERE runID=%d", table, run_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

    for(int irow = 0; irow < nrow; ++irow)
    {
    	TSQLRow* row = res->Next();

    	std::string name = getString(row, 1);
    	if(name.compare("spillCount")==0) {
    		run_header->set_spill_count(getInt(row,2));
    	}
    }

	delete res;
	return 0;
}

int ReadMySql::FillSQSpill(SQSpillMap* spill_map, TSQLServer* server,
		const int run_id, const char* table) {

	if(!spill_map) {
		LogError("!spill_map");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	//                     0      1        2
	sprintf(query, "SELECT runID, spillID, targetPos "
			" FROM %s WHERE runID=%d", table, run_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

    for(int irow = 0; irow < nrow; ++irow)
    {
    	TSQLRow* row = res->Next();

        SQSpill *spill = new SQSpill_v1();

        spill->set_run_id(getInt(row,0));
        spill->set_spill_id(getInt(row,1));
        spill->set_target_pos(getInt(row,2));

        spill_map->insert(spill);
    }

	delete res;
	return 0;
}








