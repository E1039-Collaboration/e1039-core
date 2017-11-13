/*
 * ReadMySql.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "ReadMySql.h"

#include <interfaces/SHit.h>
#include <interfaces/SHit_v1.h>
#include <interfaces/SHitMap_v1.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <TLorentzVector.h>
#include <TObjArray.h>
#include <TLeaf.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <boost/lexical_cast.hpp>

#define LogInfo(exp)		std::cout<<"INFO: "   <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogDebug(exp)		std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

#define DEBUG

ReadMySql::ReadMySql(const std::string& name) :
SubsysReco(name),
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
_event(0),
_hit_map(nullptr)
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

    sprintf(_query, "SELECT eventID FROM Event");
    _event_ids.clear();
    int nTotal = makeQueryInput();
    for(int i = 0; i < nTotal; ++i)
    {
        nextEntry();
        _event_ids.push_back(getInt(_row, 0));
    }

	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::process_event(PHCompositeNode* topNode) {

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	if(_event >= _event_ids.size()) return Fun4AllReturnCodes::ABORTRUN;

	int eventID = _event_ids[_event++];

	ReadMySql::FillHitMap(_hit_map, _input_server, eventID, "Hit");

    return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::End(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int ReadMySql::MakeNodes(PHCompositeNode* topNode) {
	PHNodeIterator iter(topNode);

	PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
	if (!eventNode) {
		LogInfo("No DST node, create one");
		eventNode = new PHCompositeNode("DST");
		topNode->addNode(eventNode);
	}

	_hit_map = new SHitMap_v1();
	PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(_hit_map,"SHitMap", "PHObject");
	eventNode->addNode(hitNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SHitMap Added");

	return Fun4AllReturnCodes::EVENT_OK;
}


int ReadMySql::GetNodes(PHCompositeNode* topNode) {
	_hit_map = findNode::getClass<SHitMap>(topNode, "SHitMap");
	if (!_hit_map) {
		LogError("!_hit_map");
		return Fun4AllReturnCodes::ABORTEVENT;
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

int ReadMySql::FillHitMap(SHitMap* hitmap, TSQLServer* server,
		const int event_id, const char* source) {

	if(!hitmap) {
		LogError("!hitmap");
		return -1;
	}

	if(!server) {
		LogError("!server");
		return -1;
	}

	char query[1000];

	//                     0      1             2          3        4       5       6          7              8           9
	sprintf(query, "SELECT hitID, detectorName, elementID, tdcTime, inTime, masked, driftTime, driftDistance, resolution, dataQuality FROM %s WHERE eventID=%d", source, event_id);

	TSQLResult *res = server->Query(query);

	int nrow = res->GetRowCount();

    for(int irow = 0; irow < nrow; ++irow)
    {
    	TSQLRow* row = res->Next();

        SHit *hit = new SHit_v1();

        hit->set_hit_id(getInt(row,0));

        hit->set_detector_id(0);
        hit->set_element_id(getInt(row,2));

        hit->set_tdc_time(getFloat(row,3));
        hit->set_drift_distance(getFloat(row,7));
        hit->set_pos(0);

        hit->set_in_time(getInt(row,4));
        hit->set_hodo_mask(getInt(row,5));
#ifdef DEBUG
        hit->identify();
#endif
        hitmap->insert(hit);
    }

	delete res;

	return 0;
}














