/*
 * SRawEventEmbed.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "SRawEventEmbed.h"

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQMCHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <ktracker/SRecEvent.h>

#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <phool/PHRandomSeed.h>

#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4HitDefs.h>
#include <g4main/PHG4VtxPoint.h>

#include <TFile.h>
#include <TTree.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <stdexcept>
#include <limits>
#include <tuple>
#include <memory>

// gsl
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <boost/lexical_cast.hpp>

using namespace std;

SRawEventEmbed::SRawEventEmbed(const std::string& name) :
SubsysReco(name),
_hit_container_type("Vector"),
_noise_rate(),
_event(0),
_hit_map(nullptr),
_hit_vector(nullptr),
_in_name("digit_016070_R007.root"),
_tin(nullptr),
_tree_entry(0),
_trigger_bit(-1),
_out_name("SRawEventEmbedEval.root")
{
}

int SRawEventEmbed::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int SRawEventEmbed::InitRun(PHCompositeNode* topNode) {

	ResetEvalVars();
	InitEvalTree();

	p_geomSvc = GeomSvc::instance();

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	TFile *fin = TFile::Open(_in_name.c_str(), "READ");
	if(!fin) {
		LogInfo("!fin");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	_tin = (TTree*) fin->Get("save");
	if(!_tin) {
		LogInfo("!_tin");
		return Fun4AllReturnCodes::ABORTRUN;
	}

	_tin->SetBranchAddress("rawEvent", &_srawevent);

	return Fun4AllReturnCodes::EVENT_OK;
}

int SRawEventEmbed::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering SRawEventEmbed::process_event: " << _event << std::endl;

	ResetEvalVars();

  if(!_hit_vector and _hit_container_type.find("Vector") == std::string::npos) {
  	LogInfo("!_hit_vector and _hit_container_type.find(\"Vector\") == std::string::npos");
  	return Fun4AllReturnCodes::ABORTRUN;
  }

  // Next entry until triggered, warp using
  while (true) {
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
    	LogInfo("_event: " << _event);
    	LogInfo("_tree_entry: " << _tree_entry);
    	LogInfo(_srawevent);
    }
    _tin->GetEntry(_tree_entry);
  	++_tree_entry;
    if(!_srawevent) {
      LogInfo("!_srawevent");
      continue;
    }
  	if((_srawevent->getTriggerBits()&_trigger_bit) > 0) break;
  	if(_tree_entry >=  _tin->GetEntries()) _tree_entry=0;
  }

  for(Hit hit : _srawevent->getAllHits()) {

//    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
//    	LogInfo("");
//    	hit.print();
//    }

		auto up_digiHit = std::unique_ptr<SQMCHit_v1> (new SQMCHit_v1());
		auto digiHit = up_digiHit.get();

		digiHit->set_hit_id(_hit_vector->size());

		digiHit->set_detector_id(hit.detectorID);
		digiHit->set_element_id(hit.elementID);
		digiHit->set_drift_distance(hit.driftDistance);

		digiHit->set_pos(p_geomSvc->getMeasurement(digiHit->get_detector_id(), digiHit->get_element_id()));

		// FIXME figure this out
		digiHit->set_in_time(hit.isInTime());
		digiHit->set_hodo_mask(hit.isHodoMask());

		digiHit->set_track_id(std::numeric_limits<int>::max());
		digiHit->set_g4hit_id(std::numeric_limits<int>::max());

		digiHit->set_truth_x(std::numeric_limits<float>::max());
		digiHit->set_truth_y(std::numeric_limits<float>::max());
		digiHit->set_truth_z(std::numeric_limits<float>::max());

//    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
//    	LogInfo("");
//    	digiHit->identify();
//    }

		_hit_vector->push_back(digiHit);
  }

  _tout->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "Leaving SRawEventEmbed::process_event: " << _event << std::endl;
  ++_event;

  return Fun4AllReturnCodes::EVENT_OK;
}

int SRawEventEmbed::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "SRawEventEmbed::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int SRawEventEmbed::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout = new TTree("T", "SRawEventEmbed");

  return 0;
}

int SRawEventEmbed::ResetEvalVars() {
  return 0;
}

int SRawEventEmbed::GetNodes(PHCompositeNode* topNode) {

  if(_hit_container_type.find("Map") != std::string::npos) {
    _hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
    if (!_hit_map) {
      LogInfo("!_hit_map");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  if(_hit_container_type.find("Vector") != std::string::npos) {
    _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
    if (!_hit_vector) {
    	LogInfo("!_hit_vector");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}







