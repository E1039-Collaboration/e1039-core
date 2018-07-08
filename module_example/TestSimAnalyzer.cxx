/*
 * TestSimAnalyzer.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "TestSimAnalyzer.h"

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
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4HitDefs.h>


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

TestSimAnalyzer::TestSimAnalyzer(const std::string& name) :
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
}

int TestSimAnalyzer::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int TestSimAnalyzer::InitRun(PHCompositeNode* topNode) {

	ResetEvalVars();
	InitEvalTree();

	p_geomSvc = GeomSvc::instance();

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	return Fun4AllReturnCodes::EVENT_OK;
}

int TestSimAnalyzer::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering TestSimAnalyzer::process_event: " << _event << std::endl;

	ResetEvalVars();

	if(_spill_map) {
    auto spill_info = _spill_map->get(_b_spill_id);
    if(spill_info) {
      _b_target_pos = spill_info->get_target_pos();
    } else {
      LogWarning("");
    }
  }

	if(_event_header) {
    _b_event_id = _event_header->get_event_id();
    _b_spill_id = _event_header->get_spill_id();
    _b_run_id   = _event_header->get_run_id();
  }

  if(_hit_vector) {
    _b_n_hits = 0;
    for(auto iter = _hit_vector->begin(); iter!= _hit_vector->end();++iter) {
    	SQHit *hit = (*iter);
      _b_hit_id[_b_n_hits]         = (*iter)->get_hit_id();
      _b_drift_distance[_b_n_hits] = (*iter)->get_drift_distance();
      _b_pos[_b_n_hits]            = (*iter)->get_pos();
      _b_detector_z[_b_n_hits]     = p_geomSvc->getPlanePosition((*iter)->get_detector_id());
      _b_detector_id[_b_n_hits]    = (*iter)->get_detector_id();
      if(_truth) {
      	//int track_id = (*iter)->get_track_id();
      	//LogDebug(track_id);
      	_b_truth_x[_b_n_hits] = (*iter)->get_truth_x();
      	_b_truth_y[_b_n_hits] = (*iter)->get_truth_y();
      	_b_truth_z[_b_n_hits] = (*iter)->get_truth_z();

      	double uVec[3] = {
      			p_geomSvc->getCostheta(hit->get_detector_id()),
						p_geomSvc->getSintheta(hit->get_detector_id()),
						0
      	};
      	_b_truth_pos[_b_n_hits] =
      			_b_truth_x[_b_n_hits]*uVec[0] + _b_truth_y[_b_n_hits]*uVec[1];

      	LogDebug("detector_id: " << _b_detector_id[_b_n_hits]);
      }
      ++_b_n_hits;
    }
  }

  LogDebug("nhits: " << _hit_vector->size());

  _tout->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "Leaving TestSimAnalyzer::process_event: " << _event << std::endl;
  ++_event;

  return Fun4AllReturnCodes::EVENT_OK;
}

int TestSimAnalyzer::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "TestSimAnalyzer::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int TestSimAnalyzer::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout = new TTree("T", "TestSimAnalyzer");
  _tout->Branch("runID",         &_b_run_id,          "runID/I");
  _tout->Branch("spillID",       &_b_spill_id,        "spillID/I");
  _tout->Branch("liveProton",    &_b_target_pos,      "liveProton/F");
  _tout->Branch("eventID",       &_b_event_id,        "eventID/I");
  _tout->Branch("nHits",         &_b_n_hits,          "nHits/I");
  _tout->Branch("hitID",         _b_hit_id,           "hitID[nHits]/I");
  _tout->Branch("detectorID",    _b_detector_id,      "detectorID[nHits]/I");
  _tout->Branch("detectorZ",     _b_detector_z,       "detectorZ[nHits]/F");
  _tout->Branch("truth_x",       _b_truth_x,          "truth_x[nHits]/F");
  _tout->Branch("truth_y",       _b_truth_y,          "truth_y[nHits]/F");
  _tout->Branch("truth_z",       _b_truth_z,          "truth_z[nHits]/F");
  _tout->Branch("truth_pos",     _b_truth_pos,        "truth_pos[nHits]/F");
  _tout->Branch("driftDistance", _b_drift_distance,   "driftDistance[nHits]/F");
  _tout->Branch("pos",           _b_pos,              "pos[nHits]/F");

  return 0;
}

int TestSimAnalyzer::ResetEvalVars() {
  _b_run_id = std::numeric_limits<int>::max();
  _b_spill_id = std::numeric_limits<int>::max();
  _b_target_pos = std::numeric_limits<float>::max();
  _b_event_id = std::numeric_limits<int>::max();
  _b_n_hits = 0;
  for(int i=0; i<10000; ++i) {
    _b_detector_id[i]    = std::numeric_limits<short>::max();
    _b_drift_distance[i] = std::numeric_limits<float>::max();
    _b_pos[i]            = std::numeric_limits<float>::max();
    _b_detector_z[i]     = std::numeric_limits<float>::max();

    _b_truth_x[i]       = std::numeric_limits<float>::max();
    _b_truth_y[i]       = std::numeric_limits<float>::max();
    _b_truth_z[i]       = std::numeric_limits<float>::max();
    _b_truth_pos[i]     = std::numeric_limits<float>::max();
  }

  return 0;
}

int TestSimAnalyzer::GetNodes(PHCompositeNode* topNode) {

  _run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!_run_header) {
    LogError("!_run_header");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }

  _spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
  if (!_spill_map) {
    LogError("!_spill_map");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }

  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogError("!_event_header");
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
  }

  _truth = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth) {
    LogError("!_truth");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }


  return Fun4AllReturnCodes::EVENT_OK;
}







