/*
 * TrkEval.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */


#include "TrkEval.h"

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

#include <boost/lexical_cast.hpp>

#define LogDebug(exp)		std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)	    std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

TrkEval::TrkEval(const std::string& name) :
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

int TrkEval::Init(PHCompositeNode* topNode) {
	return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::InitRun(PHCompositeNode* topNode) {

	ResetEvalVars();
	InitEvalTree();

	p_geomSvc = GeomSvc::instance();

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::process_event(PHCompositeNode* topNode) {

	if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
		std::cout << "Entering TrkEval::process_event: " << _event << std::endl;

	ResetEvalVars();

	if(_spill_map) {
    auto spill_info = _spill_map->get(spill_id);
    if(spill_info) {
      target_pos = spill_info->get_target_pos();
    } else {
      LogWarning("");
    }
  }

	if(_event_header) {
    event_id = _event_header->get_event_id();
    spill_id = _event_header->get_spill_id();
    run_id   = _event_header->get_run_id();
  }

  PHG4HitContainer *C1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_C1X");
  if (!C1X_hits)
  {
    cout << Name() << " Could not locate g4 hit node " << "G4HIT_C1X" << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

	std::map<int, int> parID_nhits_dc;
	std::map<int, int> parID_nhits_hodo;
	std::map<int, int> parID_nhits_prop;

	std::map<int, std::map<int, int> > parID_detid_elmid;

  typedef std::tuple<int, int> ParDetPair;
  std::map<ParDetPair, int> parID_detID_ihit;

	std::map<int, int> hitID_ihit;

  if(_hit_vector) {
    n_hits = 0;
    for(int ihit=0; ihit<_hit_vector->size(); ++ihit) {
    	SQHit *hit = _hit_vector->at(ihit);

    	int hitID = hit->get_hit_id();
    	hit_id[n_hits]         = hitID;
    	hitID_ihit[hitID]      = ihit;
      drift_distance[n_hits] = hit->get_drift_distance();
      pos[n_hits]            = hit->get_pos();
      detector_z[n_hits]     = p_geomSvc->getPlanePosition(hit->get_detector_id());
      detector_id[n_hits]    = hit->get_detector_id();

      if(_truth) {
      	int track_id = hit->get_track_id();
      	int det_id = hit->get_detector_id();
      	parID_detID_ihit[std::make_tuple(track_id, det_id)] = ihit;

      	auto detid_elmid_iter = parID_detid_elmid.find(track_id);
      	if(detid_elmid_iter != parID_detid_elmid.end()) {
      		detid_elmid_iter->second.insert(std::pair<int, int>(det_id, hit->get_element_id()));
      	} else {
      		std::map<int, int> detid_elmid;
      		detid_elmid.insert(std::pair<int, int>(det_id, hit->get_element_id()));
      		parID_detid_elmid[track_id] = detid_elmid;
      	}

      	if(hit->get_detector_id() >= 1 and hit->get_detector_id() <=30) {
        	if(parID_nhits_dc.find(track_id)!=parID_nhits_dc.end())
        		parID_nhits_dc[track_id] = parID_nhits_dc[track_id]+1;
        	else
        		parID_nhits_dc[track_id] = 1;
      	}
      	if(hit->get_detector_id() >= 31 and hit->get_detector_id() <=46) {
        	if(parID_nhits_hodo.find(track_id)!=parID_nhits_hodo.end())
        		parID_nhits_hodo[track_id] = parID_nhits_hodo[track_id]+1;
        	else
        		parID_nhits_hodo[track_id] = 1;
      	}
      	if(hit->get_detector_id() >= 47 and hit->get_detector_id() <=54) {
        	if(parID_nhits_prop.find(track_id)!=parID_nhits_prop.end())
        		parID_nhits_prop[track_id] = parID_nhits_prop[track_id]+1;
        	else
        		parID_nhits_prop[track_id] = 1;
      	}


      	truth_x[n_hits] = hit->get_truth_x();
      	truth_y[n_hits] = hit->get_truth_y();
      	truth_z[n_hits] = hit->get_truth_z();

      	double uVec[3] = {
      			p_geomSvc->getCostheta(hit->get_detector_id()),
						p_geomSvc->getSintheta(hit->get_detector_id()),
						0
      	};
      	truth_pos[n_hits] =
      			truth_x[n_hits]*uVec[0] + truth_y[n_hits]*uVec[1];

      	//LogDebug("detector_id: " << detector_id[n_hits]);
      }
      ++n_hits;
      if(n_hits>=10000) break;
    }
  }

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("ghit eval finished");

  typedef std::tuple<int, int> ParRecoPair;
  std::map<ParRecoPair, int> parID_recID_nHit;

  typedef std::tuple<int, int> TrkIDNHit;
  std::map<int, TrkIDNHit> parID_bestRecID;
  std::map<int, TrkIDNHit> recID_bestParID;

  if(_recEvent) {
  	krecstat = _recEvent->getRecStatus();

  	for(int itrack=0; itrack<_recEvent->getNTracks(); ++itrack){
  		SRecTrack recTrack = _recEvent->getTrack(itrack);

			if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
				cout << "--------- itrack: " << itrack << " ---------"<< endl;
			}

  		for(int ihit=0; ihit<recTrack.getNHits();++ihit) {
  			int hitID = recTrack.getHitIndex(ihit);

  			if(Verbosity() >= Fun4AllBase::VERBOSITY_MORE) {
  				LogDebug("hitID: " << hitID);
  			}

  			// signed hitID to hitID
  			hitID = abs(hitID);

  			//! TODO change back to map?
  			if(hitID_ihit.find(hitID)==hitID_ihit.end()) continue;

  			SQHit *hit = _hit_vector->at(hitID_ihit[hitID]);

  			if(!hit) {
    			if(Verbosity() >= Fun4AllBase::VERBOSITY_MORE) {
    				LogWarning("!hit");
    			}
  			}

  			if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
  				hit->identify();
  			}

  			// TODO better way to exclude hodo and prop hits?
  			// this is try to exclude hodo and prop hits
  			if(hit->get_detector_id() > 30) continue;

  			int parID = hit->get_track_id();
  			ParRecoPair key = std::make_tuple(parID, itrack);

      	if(parID_recID_nHit.find(key)!=parID_recID_nHit.end())
      		parID_recID_nHit[key] = parID_recID_nHit[key]+1;
      	else
      		parID_recID_nHit[key] = 1;
  		}
  	}

  	for(auto iter=parID_recID_nHit.begin();
  			iter!=parID_recID_nHit.end(); ++iter) {
  		int parID = std::get<0>(iter->first);
  		int recID = std::get<1>(iter->first);
  		int nHit  = iter->second;

    	if(parID_bestRecID.find(parID)!=parID_bestRecID.end()) {
    		int nHit_current_best  = std::get<1>(parID_bestRecID[parID]);
    		if (nHit > nHit_current_best)
    			parID_bestRecID[parID] = std::make_tuple(recID, nHit);
    	}
    	else
    		parID_bestRecID[parID] = std::make_tuple(recID, nHit);

    	if(recID_bestParID.find(recID)!=recID_bestParID.end()) {
    		int nHit_current_best  = std::get<1>(recID_bestParID[recID]);
    		if (nHit > nHit_current_best)
    			recID_bestParID[recID] = std::make_tuple(parID, nHit);
    	}
    	else
    		recID_bestParID[recID] = std::make_tuple(parID, nHit);
  	}
  }

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("parID_recID_nHit mapping finished");

  if(_truth) {
  	for(auto iter=_truth->GetPrimaryParticleRange().first;
  			iter!=_truth->GetPrimaryParticleRange().second;
  			++iter) {
  		PHG4Particle * par = iter->second;

  		pid[n_particles] = par->get_pid();

  		int vtx_id =  par->get_vtx_id();
  		PHG4VtxPoint* vtx = _truth->GetVtx(vtx_id);
  		gvx[n_particles] = vtx->get_x();
  		gvy[n_particles] = vtx->get_y();
  		gvz[n_particles] = vtx->get_z();

  		TVector3 mom(par->get_px(), par->get_py(), par->get_pz());
  		gpx[n_particles] = par->get_px();
  		gpy[n_particles] = par->get_py();
  		gpz[n_particles] = par->get_pz();
  		gpt[n_particles] = mom.Pt();
  		geta[n_particles] = mom.Eta();
  		gphi[n_particles] = mom.Phi();

  		int parID = par->get_track_id();

  		// trackID + detID -> SQHit -> PHG4Hit -> momentum
  		for(int det_id=7; det_id<=12; ++det_id) {
  			auto iter = parID_detID_ihit.find(std::make_tuple(parID, det_id));
  			if(iter != parID_detID_ihit.end()) {
					if(verbosity >= 2) {
						LogDebug("ihit: " << iter->second);
					}
  				SQHit *hit = _hit_vector->at(iter->second);
					if(verbosity >= 2) {
						hit->identify();
					}
  				if(hit) {
  					PHG4Hit* g4hit =  C1X_hits->findHit(hit->get_g4hit_id());
  					if (g4hit) {
  						if(verbosity >= 2) {
  							g4hit->identify();
  						}
							gpx_st1[n_particles] = g4hit->get_px(0)/1000.;
							gpy_st1[n_particles] = g4hit->get_py(0)/1000.;
							gpz_st1[n_particles] = g4hit->get_pz(0)/1000.;
							break;
  					}
  				}
  			}
  		}

  		gnhits[n_particles] =
  				parID_nhits_dc[parID] +
					parID_nhits_hodo[parID] +
					parID_nhits_prop[parID];

  		gndc[n_particles] = parID_nhits_dc[parID];
  		gnhodo[n_particles] = parID_nhits_hodo[parID];
  		gnprop[n_particles] = parID_nhits_prop[parID];

  		for(auto detid_elmid : parID_detid_elmid[parID]) {
  			int detid = detid_elmid.first;
  			int elmid = detid_elmid.second;
  			if(detid>=55) {
  				LogWarning("detid>=55");
  				continue;
  			}
  			gelmid[n_particles][detid] = elmid;
  		}

  		if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("particle eval finished");

  		ntruhits[n_particles] = 0;
  		if(parID_bestRecID.find(parID)!=parID_bestRecID.end()) {
  			ntruhits[n_particles] = std::get<1>(parID_bestRecID[parID]);
  			int recID = std::get<0>(parID_bestRecID[parID]);
  			SRecTrack recTrack = _recEvent->getTrack(recID);
  			charge[n_particles] = recTrack.getCharge();
  			TVector3 rec_vtx = recTrack.getTargetPos();
  			vx[n_particles]  = rec_vtx.X();
  			vy[n_particles]  = rec_vtx.Y();
  			vz[n_particles]  = rec_vtx.Z();
  			TVector3 rec_mom = recTrack.getTargetMom();
  			px[n_particles]  = rec_mom.Px();
  			py[n_particles]  = rec_mom.Py();
  			pz[n_particles]  = rec_mom.Pz();
  			pt[n_particles]  = rec_mom.Pt();
  			eta[n_particles] = rec_mom.Eta();
  			phi[n_particles] = rec_mom.Phi();

  			{
					double tx, ty, tz;
					recTrack.getMomentumSt1(tx, ty, tz);
					px_st1[n_particles] = tx;
					py_st1[n_particles] = ty;
					pz_st1[n_particles] = tz;
  			}

//  			recTrack.getMomentumSt1(
//  					&px_st1[n_particles],
//  					&py_st1[n_particles],
//  					&pz_st1[n_particles]
//						);
  		}

  		if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("single reco. eval finished");

			const double mu_mass = 0.106;
			if (abs(par->get_pid()) == 13) {
				auto iter2 = iter;
				iter2++;
				for (;iter2 != _truth->GetPrimaryParticleRange().second; ++iter2) {
					PHG4Particle* par2 = iter2->second;

					// Un-like charged
					if(par->get_pid()+par2->get_pid()!=0) continue;

					// same vtx
					if(par->get_vtx_id() != par2->get_vtx_id()) continue;

					TLorentzVector par1_mom;
					par1_mom.SetXYZM(
							par->get_px(),
							par->get_py(),
							par->get_pz(),
							mu_mass
					);

					TLorentzVector par2_mom;
					par2_mom.SetXYZM(
							par2->get_px(),
							par2->get_py(),
							par2->get_pz(),
							mu_mass
							);

					TLorentzVector vphoton = par1_mom + par2_mom;
					dimu_gpx[gndimu] = vphoton.Px();
					dimu_gpy[gndimu] = vphoton.Py();
					dimu_gpz[gndimu] = vphoton.Pz();
					dimu_gpt[gndimu] = vphoton.Pt();
					dimu_gmass[gndimu] = vphoton.M();
					dimu_geta[gndimu] = vphoton.Eta();
					dimu_gphi[gndimu] = vphoton.Phi();

					dimu_nrec[gndimu] = 0;
					if(parID_bestRecID.find(par->get_track_id())!=parID_bestRecID.end()) ++dimu_nrec[gndimu];
					if(parID_bestRecID.find(par2->get_track_id())!=parID_bestRecID.end()) ++dimu_nrec[gndimu];

					if(dimu_nrec[gndimu]==2) {
						int recID1 = std::get<0>(parID_bestRecID[par->get_track_id()]);
						int recID2 = std::get<0>(parID_bestRecID[par2->get_track_id()]);

						SRecTrack rec_trk1 = _recEvent->getTrack(recID1);
						SRecTrack rec_trk2 = _recEvent->getTrack(recID2);

						TVector3 rec_3mom1 = rec_trk1.getTargetMom();
						TVector3 rec_3mom2 = rec_trk2.getTargetMom();

						TLorentzVector rec_4mom1;
						rec_4mom1.SetXYZM(
								rec_3mom1.Px(),
								rec_3mom1.Py(),
								rec_3mom1.Pz(),
								mu_mass
						);

						TLorentzVector rec_4mom2;
						rec_4mom2.SetXYZM(
								rec_3mom2.Px(),
								rec_3mom2.Py(),
								rec_3mom2.Pz(),
								mu_mass
						);

						TLorentzVector rec_vphoton = rec_4mom1 + rec_4mom2;
						dimu_px[gndimu] = rec_vphoton.Px();
						dimu_py[gndimu] = rec_vphoton.Py();
						dimu_pz[gndimu] = rec_vphoton.Pz();
						dimu_pt[gndimu] = rec_vphoton.Pt();
						dimu_mass[gndimu] = rec_vphoton.M();
						dimu_eta[gndimu] = rec_vphoton.Eta();
						dimu_phi[gndimu] = rec_vphoton.Phi();
					}

					++gndimu;
					if(gndimu>=1000) break;
				}
			}

			if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("dimu reco. eval finished");

  		++n_particles;
  		if(n_particles>=1000) break;
  	}
  }

  _tout->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "Leaving TrkEval::process_event: " << _event << std::endl;
  ++_event;

  return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "TrkEval::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout = new TTree("T", "TrkEval");
  _tout->Branch("runID",         &run_id,          "runID/I");
  _tout->Branch("spillID",       &spill_id,        "spillID/I");
  _tout->Branch("liveProton",    &target_pos,      "liveProton/F");
  _tout->Branch("eventID",       &event_id,        "eventID/I");
  _tout->Branch("krecstat",      &krecstat,        "krecstat/I");

  _tout->Branch("nHits",         &n_hits,          "nHits/I");
  _tout->Branch("hitID",         hit_id,           "hitID[nHits]/I");
  _tout->Branch("detectorID",    detector_id,      "detectorID[nHits]/I");
  _tout->Branch("detectorZ",     detector_z,       "detectorZ[nHits]/F");
  _tout->Branch("truth_x",       truth_x,          "truth_x[nHits]/F");
  _tout->Branch("truth_y",       truth_y,          "truth_y[nHits]/F");
  _tout->Branch("truth_z",       truth_z,          "truth_z[nHits]/F");
  _tout->Branch("truth_pos",     truth_pos,        "truth_pos[nHits]/F");
  _tout->Branch("driftDistance", drift_distance,   "driftDistance[nHits]/F");
  _tout->Branch("pos",           pos,              "pos[nHits]/F");

  _tout->Branch("n_particles",   &n_particles,        "n_particles/I");
  _tout->Branch("pid",           pid,                 "pid[n_particles]/I");
  _tout->Branch("gvx",           gvx,                 "gvx[n_particles]/F");
  _tout->Branch("gvy",           gvy,                 "gvy[n_particles]/F");
  _tout->Branch("gvz",           gvz,                 "gvz[n_particles]/F");
  _tout->Branch("gpx",           gpx,                 "gpx[n_particles]/F");
  _tout->Branch("gpy",           gpy,                 "gpy[n_particles]/F");
  _tout->Branch("gpz",           gpz,                 "gpz[n_particles]/F");
  _tout->Branch("gpt",           gpt,                 "gpt[n_particles]/F");
  _tout->Branch("geta",          geta,                "geta[n_particles]/F");
  _tout->Branch("gphi",          gphi,                "gphi[n_particles]/F");
  _tout->Branch("gpx_st1",       gpx_st1,             "gpx_st1[n_particles]/F");
  _tout->Branch("gpy_st1",       gpy_st1,             "gpy_st1[n_particles]/F");
  _tout->Branch("gpz_st1",       gpz_st1,             "gpz_st1[n_particles]/F");
  _tout->Branch("gnhits",        gnhits,              "gnhits[n_particles]/I");
  _tout->Branch("gndc",          gndc,                "gndc[n_particles]/I");
  _tout->Branch("gnhodo",        gnhodo,              "gnhodo[n_particles]/I");
  _tout->Branch("gnprop",        gnprop,              "gnprop[n_particles]/I");
  _tout->Branch("gelmid",        gelmid,              "gelmid[n_particles][54]/I");

  _tout->Branch("ntruhits",      ntruhits,            "ntruhits[n_particles]/I");
  _tout->Branch("charge",        charge,              "charge[n_particles]/I");
  _tout->Branch("vx",            vx,                  "vx[n_particles]/F");
  _tout->Branch("vy",            vy,                  "vy[n_particles]/F");
  _tout->Branch("vz",            vz,                  "vz[n_particles]/F");
  _tout->Branch("px",            px,                  "px[n_particles]/F");
  _tout->Branch("py",            py,                  "py[n_particles]/F");
  _tout->Branch("pz",            pz,                  "pz[n_particles]/F");
  _tout->Branch("pt",            pt,                  "pt[n_particles]/F");
  _tout->Branch("eta",           eta,                 "eta[n_particles]/F");
  _tout->Branch("phi",           phi,                 "phi[n_particles]/F");
  _tout->Branch("px_st1",        px_st1,              "px_st1[n_particles]/F");
  _tout->Branch("py_st1",        py_st1,              "py_st1[n_particles]/F");
  _tout->Branch("pz_st1",        pz_st1,              "pz_st1[n_particles]/F");

  _tout->Branch("gndimu",        &gndimu,              "gndimu/I");
  _tout->Branch("dimu_gpx",      dimu_gpx,             "dimu_gpx[gndimu]/F");
  _tout->Branch("dimu_gpy",      dimu_gpy,             "dimu_gpy[gndimu]/F");
  _tout->Branch("dimu_gpz",      dimu_gpz,             "dimu_gpz[gndimu]/F");
  _tout->Branch("dimu_gpt",      dimu_gpt,             "dimu_gpt[gndimu]/F");
  _tout->Branch("dimu_gmass",    dimu_gmass,           "dimu_gmass[gndimu]/F");
  _tout->Branch("dimu_geta",     dimu_geta,            "dimu_geta[gndimu]/F");
  _tout->Branch("dimu_gphi",     dimu_gphi,            "dimu_gphi[gndimu]/F");

  _tout->Branch("dimu_nrec",     dimu_nrec,            "dimu_nrec[gndimu]/I");
  _tout->Branch("dimu_px",       dimu_px,              "dimu_px[gndimu]/F");
  _tout->Branch("dimu_py",       dimu_py,              "dimu_py[gndimu]/F");
  _tout->Branch("dimu_pz",       dimu_pz,              "dimu_pz[gndimu]/F");
  _tout->Branch("dimu_pt",       dimu_pt,              "dimu_pt[gndimu]/F");
  _tout->Branch("dimu_mass",     dimu_mass,            "dimu_mass[gndimu]/F");
  _tout->Branch("dimu_eta",      dimu_eta,             "dimu_eta[gndimu]/F");
  _tout->Branch("dimu_phi",      dimu_phi,             "dimu_phi[gndimu]/F");

  return 0;
}

int TrkEval::ResetEvalVars() {
  run_id = std::numeric_limits<int>::max();
  spill_id = std::numeric_limits<int>::max();
  target_pos = std::numeric_limits<float>::max();
  event_id = std::numeric_limits<int>::max();
  krecstat = std::numeric_limits<int>::max();

  n_hits = 0;
  for(int i=0; i<10000; ++i) {
    detector_id[i]    = std::numeric_limits<short>::max();
    drift_distance[i] = std::numeric_limits<float>::max();
    pos[i]            = std::numeric_limits<float>::max();
    detector_z[i]     = std::numeric_limits<float>::max();

    truth_x[i]       = std::numeric_limits<float>::max();
    truth_y[i]       = std::numeric_limits<float>::max();
    truth_z[i]       = std::numeric_limits<float>::max();
    truth_pos[i]     = std::numeric_limits<float>::max();
  }

  n_particles = 0;
  for(int i=0; i<1000; ++i) {
    pid[i]        = std::numeric_limits<float>::max();
    gvx[i]        = std::numeric_limits<float>::max();
    gvy[i]        = std::numeric_limits<float>::max();
    gvz[i]        = std::numeric_limits<float>::max();
    gpx[i]        = std::numeric_limits<float>::max();
    gpy[i]        = std::numeric_limits<float>::max();
    gpz[i]        = std::numeric_limits<float>::max();
    gpt[i]        = std::numeric_limits<float>::max();
    geta[i]       = std::numeric_limits<float>::max();
    gphi[i]       = std::numeric_limits<float>::max();
    gnhits[i]     = std::numeric_limits<int>::max();
    gpx_st1[i]    = std::numeric_limits<float>::max();
    gpy_st1[i]    = std::numeric_limits<float>::max();
    gpz_st1[i]    = std::numeric_limits<float>::max();
    gndc[i]       = std::numeric_limits<int>::max();
    gnhodo[i]     = std::numeric_limits<int>::max();
    gnprop[i]     = std::numeric_limits<int>::max();

    for(int j=0; j<55; ++j) {
    	gelmid[i][j] = std::numeric_limits<int>::max();
    }

    ntruhits[i]   = std::numeric_limits<int>::max();
    charge[i]     = std::numeric_limits<int>::max();
    vx[i]         = std::numeric_limits<float>::max();
    vy[i]         = std::numeric_limits<float>::max();
    vz[i]         = std::numeric_limits<float>::max();
    px[i]         = std::numeric_limits<float>::max();
    py[i]         = std::numeric_limits<float>::max();
    pz[i]         = std::numeric_limits<float>::max();
    pt[i]         = std::numeric_limits<float>::max();
    eta[i]        = std::numeric_limits<float>::max();
    phi[i]        = std::numeric_limits<float>::max();
    px_st1[i]     = std::numeric_limits<float>::max();
    py_st1[i]     = std::numeric_limits<float>::max();
    pz_st1[i]     = std::numeric_limits<float>::max();
  }

  gndimu = 0;
  for(int i=0; i<1000; ++i) {
  	dimu_gpx[i]        = std::numeric_limits<float>::max();
  	dimu_gpy[i]        = std::numeric_limits<float>::max();
  	dimu_gpz[i]        = std::numeric_limits<float>::max();
  	dimu_gpt[i]        = std::numeric_limits<float>::max();
  	dimu_gmass[i]      = std::numeric_limits<float>::max();
  	dimu_geta[i]       = std::numeric_limits<float>::max();
  	dimu_gphi[i]       = std::numeric_limits<float>::max();

  	dimu_nrec[i]       = 0;
  	dimu_px[i]         = std::numeric_limits<float>::max();
  	dimu_py[i]         = std::numeric_limits<float>::max();
  	dimu_pz[i]         = std::numeric_limits<float>::max();
  	dimu_pt[i]         = std::numeric_limits<float>::max();
  	dimu_mass[i]       = std::numeric_limits<float>::max();
  	dimu_eta[i]        = std::numeric_limits<float>::max();
  	dimu_phi[i]        = std::numeric_limits<float>::max();
  }

  return 0;
}

int TrkEval::GetNodes(PHCompositeNode* topNode) {

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
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
  if (!_recEvent) {
    LogError("!_recEvent");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}







