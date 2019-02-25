/*
 * RndmEmbed.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "RndmEmbed.h"

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

// gsl
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

#include <boost/lexical_cast.hpp>

using namespace std;

RndmEmbed::RndmEmbed(const std::string& name) :
SubsysReco(name),
_hit_container_type("Vector"),
_noise_rate(),
_event(0),
_hit_map(nullptr),
_hit_vector(nullptr),
_out_name("RndmEmbedEval.root")
{
}

int RndmEmbed::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int RndmEmbed::InitRun(PHCompositeNode* topNode) {

	ResetEvalVars();
	InitEvalTree();

	p_geomSvc = GeomSvc::instance();

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	return Fun4AllReturnCodes::EVENT_OK;
}

int RndmEmbed::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering RndmEmbed::process_event: " << _event << std::endl;

	ResetEvalVars();

  if(!_hit_vector and _hit_container_type.find("Vector") == std::string::npos) {
  	LogInfo("!_hit_vector and _hit_container_type.find(\"Vector\") == std::string::npos");
  	return Fun4AllReturnCodes::ABORTRUN;
  }

#ifndef __CINT__

  gsl_rng* RandomGenerator = gsl_rng_alloc(gsl_rng_mt19937);
  unsigned int seed = PHRandomSeed();  // fixed seed is handled in this funtcion
  //seed = 128;
	//std::cout << Name() << " random seed: " << seed << std::endl;
  gsl_rng_set(RandomGenerator, seed);

  for (auto i : _noise_rate) {
  	auto detector_name = i.first;
  	auto noise_rate = i.second;

  	auto detector_id = p_geomSvc->getDetectorID(detector_name);
  	if(detector_id==0) {
  		LogInfo("detector_id==0");
  		continue;
  	}

  	Plane plane = p_geomSvc->getPlane(detector_id);

  	for( int element_id=1; element_id <= plane.nElements; ++element_id) {
  		bool acc = gsl_ran_flat(RandomGenerator, 0, 1) < noise_rate ? true : false;
  		if(!acc) continue;

  		{
				double drift = plane.spacing * gsl_ran_flat(RandomGenerator, -1, 1);

				auto up_digiHit = std::unique_ptr<SQMCHit_v1> (new SQMCHit_v1());
				auto digiHit = up_digiHit.get();

				digiHit->set_hit_id(_hit_vector->size());

				digiHit->set_detector_id(detector_id);
				digiHit->set_element_id(element_id);
				digiHit->set_drift_distance(drift);

				digiHit->set_pos(p_geomSvc->getMeasurement(digiHit->get_detector_id(), digiHit->get_element_id()));

				// FIXME figure this out
				digiHit->set_in_time(1);
				digiHit->set_hodo_mask(1);

				digiHit->set_track_id(std::numeric_limits<int>::max());
				digiHit->set_g4hit_id(std::numeric_limits<int>::max());

				digiHit->set_truth_x(std::numeric_limits<float>::max());
				digiHit->set_truth_y(std::numeric_limits<float>::max());
				digiHit->set_truth_z(std::numeric_limits<float>::max());

				_hit_vector->push_back(digiHit);
  		}

  		{
				double drift = plane.spacing * gsl_ran_flat(RandomGenerator, -1, 1);

				auto up_digiHit = std::unique_ptr<SQMCHit_v1> (new SQMCHit_v1());
				auto digiHit = up_digiHit.get();

				digiHit->set_hit_id(_hit_vector->size());

				digiHit->set_detector_id(detector_id+1);
				digiHit->set_element_id(element_id);
				digiHit->set_drift_distance(drift);

				digiHit->set_pos(p_geomSvc->getMeasurement(digiHit->get_detector_id(), digiHit->get_element_id()));

				// FIXME figure this out
				digiHit->set_in_time(1);
				digiHit->set_hodo_mask(1);

				digiHit->set_track_id(std::numeric_limits<int>::max());
				digiHit->set_g4hit_id(std::numeric_limits<int>::max());

				digiHit->set_truth_x(std::numeric_limits<float>::max());
				digiHit->set_truth_y(std::numeric_limits<float>::max());
				digiHit->set_truth_z(std::numeric_limits<float>::max());

				_hit_vector->push_back(digiHit);
  		}

  	}
  }

  gsl_rng_free(RandomGenerator);

#endif //__CINT__

  _tout->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "Leaving RndmEmbed::process_event: " << _event << std::endl;
  ++_event;

  return Fun4AllReturnCodes::EVENT_OK;
}

int RndmEmbed::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "RndmEmbed::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int RndmEmbed::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout = new TTree("T", "RndmEmbed");

  return 0;
}

int RndmEmbed::ResetEvalVars() {
  return 0;
}

int RndmEmbed::GetNodes(PHCompositeNode* topNode) {

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







