/*
 * KalmanFastTrackingWrapper.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "KalmanFastTrackingWrapper.h"

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>


#include <TFile.h>
#include <TTree.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <limits>
#include <boost/lexical_cast.hpp>

#define LogDebug(exp)		std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

KalmanFastTrackingWrapper::KalmanFastTrackingWrapper(const std::string& name) :
SubsysReco(name),
_hit_container_type("Vector"),
_event(0),
_run_header(nullptr),
_spill_map(nullptr),
_event_header(nullptr),
_hit_map(nullptr),
_hit_vector(nullptr),
_out_name("eval.root")
{
	p_jobOptsSvc = new JobOptsSvc;
	//p_jobOptsSvc = JobOptsSvc::instance();

	p_jobOptsSvc->init("default.opts");

	fastfinder = new KalmanFastTracking();

	ResetEvalVars();
	InitEvalTree();

}

int KalmanFastTrackingWrapper::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::InitRun(PHCompositeNode* topNode) {

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	return Fun4AllReturnCodes::EVENT_OK;
}

SRawEvent* KalmanFastTrackingWrapper::BuildSRawEvent() {
	SRawEvent* sraw_event = new SRawEvent();


	int run_id   = _event_header->get_run_id();
	int spill_id = _event_header->get_spill_id();
	int event_id = _event_header->get_event_id();

	sraw_event->setEventInfo(run_id, spill_id, event_id);

  int triggers[10];
  for(int i = SQEvent::NIM1; i <= SQEvent::MATRIX5; ++i)
  {
      triggers[i] = _event_header->get_trigger(static_cast<SQEvent::TriggerMask>(i));
  }
  sraw_event->setTriggerBits(triggers);

  //Get target position
  //sprintf(query, "SELECT targetPos FROM Spill WHERE spillID=%d", spillID);

  //Get beam information - QIE

  //Get trigger hits - TriggerHit

	for(auto iter = _hit_vector->begin(); iter!= _hit_vector->end();++iter) {
    SQHit *sq_hit = (*iter);

    Hit h;
    h.index = sq_hit->get_hit_id();
    h.detectorID = sq_hit->get_detector_id();
    h.elementID = sq_hit->get_element_id();
    h.tdcTime = sq_hit->get_tdc_time();
    h.driftDistance = sq_hit->get_drift_distance();
    h.pos = sq_hit->get_pos();

    //TODO Hit:InTime
    //if(getInt(5, 0) > 0) h.setInTime();

    //TODO Hit:masked
    //if(getInt(6, 0) > 0) h.setHodoMask();

    // TODO calibration
//    if(p_geomSvc->isCalibrationLoaded())
//    {
//        if((h.detectorID >= 1 && h.detectorID <= nChamberPlanes) || (h.detectorID >= nChamberPlanes+nHodoPlanes+1))
//        {
//            h.setInTime(p_geomSvc->isInTime(h.detectorID, h.tdcTime));
//            if(h.isInTime()) h.driftDistance = p_geomSvc->getDriftDistance(h.detectorID, h.tdcTime);
//        }
//    }

    sraw_event->insertHit(h);
	}

	sraw_event->reIndex(true);
}

int KalmanFastTrackingWrapper::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering KalmanFastTrackingWrapper::process_event: " << _event << std::endl;

	ResetEvalVars();

	if(!_event_header) {
		LogDebug("!_event_header");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	if(!_spill_map) {
		LogDebug("!_spill_map");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	if(!_hit_vector) {
		LogDebug("!_hit_vector");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	SRawEvent* sraw_event = BuildSRawEvent();
	_rawEvent = sraw_event;

	_recEvent = new SRecEvent();

	_recEvent->setRecStatus(fastfinder->setRawEvent(sraw_event));

  _recEvent->setRawEvent(_rawEvent);

  std::list<Tracklet>& rec_tracklets = fastfinder->getFinalTracklets();

  for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
  {
      iter->calcChisq();
      iter->print();

      SRecTrack recTrack = iter->getSRecTrack();
      _recEvent->insertTrack(recTrack);
  }


	_tout->Fill();

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Leaving KalmanFastTrackingWrapper::process_event: " << _event << std::endl;
	++_event;

	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::End(PHCompositeNode* topNode) {
	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "KalmanFastTrackingWrapper::End" << std::endl;

	delete fastfinder;

	PHTFileServer::get().cd(_out_name.c_str());
	_tout->Write();

	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::InitEvalTree() {
	PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

	_tout = new TTree("save", "save");
	_tout->Branch("rawEvent", &_rawEvent, 256000, 99);
	_tout->Branch("recEvent", &_recEvent, 256000, 99);

	return 0;
}

int KalmanFastTrackingWrapper::ResetEvalVars() {
	_rawEvent = nullptr;
	_recEvent = nullptr;

	return 0;
}

int KalmanFastTrackingWrapper::GetNodes(PHCompositeNode* topNode) {

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
