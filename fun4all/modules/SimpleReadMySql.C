/*
 * SimpleReadMySql.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "SimpleReadMySql.h"

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

SimpleReadMySql::SimpleReadMySql(const std::string& name) :
SubsysReco(name),
_server_name("seaquestdb01.fnal.gov"),
_port(3310),
_user_name("seaguest"),
_password("qqbar2mu+mu-"),
_input_shcema("run_013899_R007"),
_table_name("kHit"),
_run_id(-1),
_spill_id(-1),
inputServer(nullptr),
res(nullptr),
row(nullptr),
_event(0),
_hit_map(nullptr)
{
	eventIDs.clear();
}

int SimpleReadMySql::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int SimpleReadMySql::InitRun(PHCompositeNode* topNode) {

    char serverUrl[200];
    sprintf(serverUrl, "mysql://%s:%d", _server_name.c_str(), _port);
    LogDebug(serverUrl);
    inputServer = TSQLServer::Connect(serverUrl, _user_name.c_str(), _password.c_str());
    LogDebug("");
    if(!inputServer) {
    	if(Verbosity() >= Fun4AllBase::VERBOSITY_QUIET)
    		LogError("!inputServer");
    	return Fun4AllReturnCodes::ABORTRUN;
    }

    //check the essential info
    sprintf(query, "USE %s", _input_shcema.c_str());
    if(!inputServer->Exec(query))
    {
        std::cout << "MySQLSvc: working schema does not exist! Will exit..." << std::endl;
        return Fun4AllReturnCodes::ABORTRUN;
    }

    int ret = MakeNodes(topNode);
    if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

    sprintf(query, "SELECT eventID FROM Event");
    eventIDs.clear();
    int nTotal = makeQueryInput();
    for(int i = 0; i < nTotal; ++i)
    {
        nextEntry();
        eventIDs.push_back(getInt(0));
    }

	return Fun4AllReturnCodes::EVENT_OK;
}

int SimpleReadMySql::process_event(PHCompositeNode* topNode) {

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	if(_event >= eventIDs.size()) return Fun4AllReturnCodes::ABORTRUN;

	int eventID = eventIDs[_event++];

//	sprintf(query, "SELECT hitID,elementID,tdcTime,driftDistance,detectorName,inTime,masked FROM Hit WHERE (detectorName LIKE 'D%%' "
//            "OR detectorName LIKE 'H%%' OR detectorName LIKE 'P%%') AND eventID=%d", eventID);

	sprintf(query, "SELECT hitID, driftDistance FROM kHit WHERE eventID=%d", eventID);

	int nHits = makeQueryInput();

    for(int i = 0; i < nHits; ++i)
    {
        nextEntry();
        SHit *hit = new SHit_v1();
        hit->set_detector_name("Bla");
        hit->set_element_id(0);
        hit->set_id(getInt(0));
        hit->set_drift_distance(getFloat(1));

        if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
        	hit->identify();
        }

        _hit_map->insert(hit);
    }

    return Fun4AllReturnCodes::EVENT_OK;
}

int SimpleReadMySql::End(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int SimpleReadMySql::MakeNodes(PHCompositeNode* topNode) {
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


int SimpleReadMySql::GetNodes(PHCompositeNode* topNode) {
	_hit_map = findNode::getClass<SHitMap>(topNode, "SHitMap");
	if (!_hit_map) {
		LogError("!_hit_map");
		return Fun4AllReturnCodes::ABORTEVENT;
	}
	return Fun4AllReturnCodes::EVENT_OK;
}



int SimpleReadMySql::makeQueryInput() {

    if(inputServer == NULL) return 0;

    if(res != NULL) delete res;
    res = inputServer->Query(query);

    if(res != NULL) return res->GetRowCount();
    return 0;
}


bool SimpleReadMySql::nextEntry() {
    if(res == NULL) return false;

    if(row != NULL) delete row;
    row = res->Next();

    if(row != NULL) return true;
    return false;
}


int SimpleReadMySql::getInt(int id, int default_val)
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

float SimpleReadMySql::getFloat(int id, float default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<float>(row->GetField(id));
}

double SimpleReadMySql::getDouble(int id, double default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return boost::lexical_cast<double>(row->GetField(id));
}

std::string SimpleReadMySql::getString(int id, std::string default_val)
{
    if(row->GetField(id) == NULL)
    {
        return default_val;
    }

    return std::string(row->GetField(id));
}
















