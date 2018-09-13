/*
 * MainDAQDecoder.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "MainDAQDecoder.h"

#include <event/Event.h>
#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>

#define LogInfo(exp)		std::cout<<"INFO: "   <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

MainDAQDecoder::MainDAQDecoder(const std::string& name) :
SubsysReco(name),
_hit_map(nullptr)
{
}

int MainDAQDecoder::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int MainDAQDecoder::InitRun(PHCompositeNode* topNode) {

	return Fun4AllReturnCodes::EVENT_OK;
}

int MainDAQDecoder::process_event(PHCompositeNode* topNode) {

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	int* event_words = _evio_event->get_words();

	/*
	 * TODO fill in the real process
	 */

	return Fun4AllReturnCodes::EVENT_OK;
}

int MainDAQDecoder::End(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int MainDAQDecoder::MakeNodes(PHCompositeNode* topNode) {
	PHNodeIterator iter(topNode);

	PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
	if (!eventNode) {
		LogInfo("No DST node, create one");
		eventNode = new PHCompositeNode("DST");
		topNode->addNode(eventNode);
	}

	_hit_map = new SQHitMap_v1();
	PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(_hit_map,"SQHitMap", "PHObject");
	eventNode->addNode(hitNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SQHitMap Added");

	return Fun4AllReturnCodes::EVENT_OK;
}


int MainDAQDecoder::GetNodes(PHCompositeNode* topNode) {

	_evio_event = findNode::getClass<Event>(topNode, "EVIO");
	if (!_evio_event) {
		LogInfo("!_evio_event");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	_hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
	if (!_hit_map) {
		LogInfo("!_hit_map");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	return Fun4AllReturnCodes::EVENT_OK;
}















