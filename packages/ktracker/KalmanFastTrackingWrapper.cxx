/*
 * KalmanFastTrackingWrapper.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "KalmanFastTrackingWrapper.h"
#include "KalmanFastTracking.h"
#include "EventReducer.h"

#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHFieldUtility.h>

#include <phgeom/PHGeomUtility.h>

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
#include <cassert>
#include <stdexcept>
#include <limits>
#include <memory>

#include <boost/lexical_cast.hpp>

#define LogDebug(exp)		    std::cout<<"DEBUG: "  <<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		    std::cout<<"ERROR: "  <<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FUNCTION__<<": "<<__LINE__<<": "<< exp << std::endl

//#define _DEBUG_ON

using namespace std;

KalmanFastTrackingWrapper::KalmanFastTrackingWrapper(const std::string& name) :
SubsysReco(name),
_hit_container_type("Vector"),
_event(0),
_run_header(nullptr),
_spill_map(nullptr),
_event_header(nullptr),
_hit_map(nullptr),
_hit_vector(nullptr),
_triggerhit_vector(nullptr),
_out_name("ktracker_eval.root")
{
	//p_jobOptsSvc = new JobOptsSvc();
	p_jobOptsSvc = JobOptsSvc::instance();
	p_jobOptsSvc->init("default.opts");
	LogDebug(p_jobOptsSvc->m_geomVersion);
}

int KalmanFastTrackingWrapper::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::InitRun(PHCompositeNode* topNode) {

	ResetEvalVars();
	InitEvalTree();

	int ret = MakeNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	ret = InitField(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	ret = InitGeom(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	PHField* field = PHFieldUtility::GetFieldMapNode(nullptr, topNode);
	assert(field);

	/// init KalmanFastTracking
	fastfinder = new KalmanFastTracking(field, _t_geo_manager);
	fastfinder->Verbosity(verbosity);

  TString opt = "aoc";      //turn on after pulse removal, out of time removal, and cluster removal
  if(p_jobOptsSvc->m_enableTriggerMask) opt = opt + "t";
  if(p_jobOptsSvc->m_sagittaReducer) opt = opt + "s";
  if(p_jobOptsSvc->m_updateAlignment) opt = opt + "e";
  if(p_jobOptsSvc->m_hodomask) opt = opt + "h";
  if(p_jobOptsSvc->m_mergeHodo) opt = opt + "m";
  if(p_jobOptsSvc->m_realization) opt = opt + "r";

  eventReducer = new EventReducer(opt);

	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::InitField(PHCompositeNode *topNode)
{
  if (verbosity > 1) cout << "PHG4Reco::InitField - create magnetic field setup" << endl;

  unique_ptr<PHFieldConfig> default_field_cfg(nullptr);

  default_field_cfg.reset(new PHFieldConfig_v3(p_jobOptsSvc->m_fMagFile, p_jobOptsSvc->m_kMagFile));

  if (verbosity > 1) cout << "PHG4Reco::InitField - create magnetic field setup" << endl;

  PHField * phfield = PHFieldUtility::GetFieldMapNode(default_field_cfg.get(), topNode, 0);
  assert(phfield);

  return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::InitGeom(PHCompositeNode *topNode)
{
	PHGeomUtility::ImportGeomFile(topNode, _geom_file_name);
	_t_geo_manager = PHGeomUtility::GetTGeoManager(topNode);

  return Fun4AllReturnCodes::EVENT_OK;
}

SRawEvent* KalmanFastTrackingWrapper::BuildSRawEvent() {
	SRawEvent* sraw_event = new SRawEvent();

#ifdef _DEBUG_ON
	LogDebug("Start: ");
#endif

	int run_id   = 0;
	int spill_id = 0;
	int event_id = _event;

	if(_event_header) {
		run_id   = _event_header->get_run_id();
		spill_id = _event_header->get_spill_id();
		event_id = _event_header->get_event_id();
	}

	sraw_event->setEventInfo(run_id, spill_id, event_id);

  int triggers[10];
  for(int i = SQEvent::NIM1; i <= SQEvent::MATRIX5; ++i)
  {
		if(_event_header)
			triggers[i] = _event_header->get_trigger(static_cast<SQEvent::TriggerMask>(i));
		else
			triggers[i] = 1;
  }
  sraw_event->setTriggerBits(triggers);

  //Get target position

  if(_spill_map) {
		SQSpill *spill = _spill_map->get(spill_id);
		if(!spill) {
			if(verbosity >= 2) LogError("No Spill Info for ID: ") << spill_id << endl;
		}
		sraw_event->setTargetPos(spill->get_target_pos());
  } else {
  	sraw_event->setTargetPos(1);
  }

  //Get beam information - QIE

  //Get trigger hits - TriggerHit
  if(_triggerhit_vector) {
	  for(auto iter = _triggerhit_vector->begin(); iter!= _triggerhit_vector->end();++iter) {
      SQHit *sq_hit = (*iter);

      Hit h;
      h.index = sq_hit->get_hit_id();
      h.detectorID = sq_hit->get_detector_id();
      h.elementID = sq_hit->get_element_id();
      h.tdcTime = sq_hit->get_tdc_time();
      h.driftDistance = 0;
      h.pos = sq_hit->get_pos();

      if(sq_hit->is_in_time()) h.setInTime();

      sraw_event->insertTriggerHit(h);
	  }
  }

	for(auto iter = _hit_vector->begin(); iter!= _hit_vector->end();++iter) {
    SQHit *sq_hit = (*iter);

    Hit h;
    h.index = sq_hit->get_hit_id();
    h.detectorID = sq_hit->get_detector_id();
    h.elementID = sq_hit->get_element_id();
    h.tdcTime = sq_hit->get_tdc_time();
    h.driftDistance = sq_hit->get_drift_distance();
    h.pos = sq_hit->get_pos();

    if(sq_hit->is_in_time()) h.setInTime();

    //TODO Hit:masked
    if(sq_hit->is_hodo_mask()) h.setHodoMask();

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

    // FIXME just for the meeting, figure this out fast!
    if(!_triggerhit_vector and h.detectorID >= 31) {
    	sraw_event->insertTriggerHit(h);
    }
	}

	sraw_event->reIndex(true);

#ifdef _DEBUG_ON
	LogDebug("End: ");
#endif

	return sraw_event;
}

int KalmanFastTrackingWrapper::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering KalmanFastTrackingWrapper::process_event: " << _event << std::endl;

		ResetEvalVars();

	if(!_event_header) {
		if(Verbosity() > 2) LogDebug("!_event_header");
		//return Fun4AllReturnCodes::ABORTRUN;
	}


#ifdef _DEBUG_ON
	if(_event_header) {
		cout
		<< "runID: " << _event_header->get_run_id()
		<< " | spillID: " << _event_header->get_spill_id()
		<< " | eventID: " << _event_header->get_event_id()
		<< endl;
	}
#endif

  if(!_spill_map) {
  	if(Verbosity() > 2) LogDebug("!_spill_map");
		//return Fun4AllReturnCodes::ABORTRUN;
	}

	if(!_hit_vector) {
		LogDebug("!_hit_vector");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	auto up_raw_event = std::unique_ptr<SRawEvent>(BuildSRawEvent());
	SRawEvent* sraw_event = up_raw_event.get();

  // TODO temp solution
  if(_event_header) {
	  eventReducer->reduceEvent(sraw_event);
  }

	_rawEvent = sraw_event;

	//auto up_recEvent = std::unique_ptr<SRecEvent>(new SRecEvent());
	//_recEvent = up_recEvent.get();

	_recEvent->setRecStatus(fastfinder->setRawEvent(sraw_event));

  if(verbosity >= 2)
  	fastfinder->printTimers();

  _recEvent->setRawEvent(_rawEvent);

  std::list<Tracklet>& rec_tracklets = fastfinder->getFinalTracklets();

  for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
  {
		iter->calcChisq();
#ifdef _DEBUG_ON
		iter->print();
#endif

#ifndef _ENABLE_KF
		SRecTrack recTrack = iter->getSRecTrack();
#ifdef _DEBUG_ON
		recTrack.print();
#endif
		_recEvent->insertTrack(recTrack);
#endif
  }

#ifdef _ENABLE_KF
	std::list<SRecTrack>& rec_tracks = fastfinder->getSRecTracks();
	for(std::list<SRecTrack>::iterator iter = rec_tracks.begin(); iter != rec_tracks.end(); ++iter)
	{
#ifdef _DEBUG_ON
		iter->print();
#endif
		_recEvent->insertTrack(*iter);
	}
#endif


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

	_tout = new TTree("T", "save");
	_tout->Branch("rawEvent", &_rawEvent, 256000, 99);
	_tout->Branch("recEvent", &_recEvent, 256000, 99);

	return 0;
}

int KalmanFastTrackingWrapper::ResetEvalVars() {
	//_rawEvent = nullptr;
	//_recEvent = nullptr;

	return 0;
}

int KalmanFastTrackingWrapper::MakeNodes(PHCompositeNode* topNode) {

	PHNodeIterator iter(topNode);

	PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
	if (!eventNode) {
		LogInfo("No DST node, create one");
		eventNode = new PHCompositeNode("DST");
		topNode->addNode(eventNode);
	}

	_recEvent = new SRecEvent();
	PHIODataNode<PHObject>* recEventNode = new PHIODataNode<PHObject>(_recEvent,"SRecEvent", "PHObject");
	eventNode->addNode(recEventNode);
	if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
		LogInfo("DST/SRecEvent Added");

	return Fun4AllReturnCodes::EVENT_OK;
}

int KalmanFastTrackingWrapper::GetNodes(PHCompositeNode* topNode) {

	_run_header = findNode::getClass<SQRun>(topNode, "SQRun");
	if (!_run_header) {
		if(Verbosity() > 2) LogError("!_run_header");
		//return Fun4AllReturnCodes::ABORTEVENT;
	}

	_spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
	if (!_spill_map) {
		if(Verbosity() > 2) LogError("!_spill_map");
		//return Fun4AllReturnCodes::ABORTEVENT;
	}

	_event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
	if (!_event_header) {
		if(Verbosity() > 2) LogError("!_event_header");
		//return Fun4AllReturnCodes::ABORTEVENT;
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

		_triggerhit_vector = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
		if (!_triggerhit_vector) {
			if(Verbosity() > 2) LogError("!_triggerhit_vector");
			//return Fun4AllReturnCodes::ABORTEVENT;
		}
	}

	_recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
	if (!_recEvent) {
		if(Verbosity() > 2) LogError("!_recEvent");
		return Fun4AllReturnCodes::ABORTEVENT;
	}

	return Fun4AllReturnCodes::EVENT_OK;
}
