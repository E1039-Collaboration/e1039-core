/**
 * \class TrkEval
 * \brief General purposed evaluation module
 * \author Haiwang Yu, yuhw@nmsu.edu
 *
 * Created: 08-27-2018
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
	int ret = Fun4AllReturnCodes::ABORTRUN;

	ret = TruthEval(topNode);
	if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	if(_recEvent) {
		ret = RecoEval(topNode);
		if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;
	}

	++_event;

	return ret;
}

int TrkEval::RecoEval(PHCompositeNode* topNode)
{
	if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
		std::cout << "Entering TrkEval::RecoEval: " << _event << std::endl;

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
    event_id    = _event_header->get_event_id();
    emu_trigger = _event_header->get_trigger();
    spill_id    = _event_header->get_spill_id();
    run_id      = _event_header->get_run_id();
  }

  PHG4HitContainer *D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D0X");
  if (!D1X_hits)
    D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D1X");

  if (!D1X_hits)
  {
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
        cout << Name() << " Could not locate g4 hit node " << "G4HIT_D0X or G4HIT_D1X" << endl;
  }

	std::map<int, int> parID_nhits_dc;
	std::map<int, int> parID_nhits_hodo;
	std::map<int, int> parID_nhits_prop;

	std::map<int, std::map<int, int> > parID_detid_elmid;

  typedef std::tuple<int, int> ParDetPair;
  std::map<ParDetPair, int> parID_detID_ihit;

  std::map<int, int> hitID_ihit;

  // If using vector, index it first
  if(_hit_vector) {
    for(int ihit=0; ihit<_hit_vector->size(); ++ihit) {
    	SQHit *hit = _hit_vector->at(ihit);
    	int hitID = hit->get_hit_id();
    	hitID_ihit[hitID]      = ihit;

      if(_truth) {
      	int track_id = hit->get_track_id();
      	int det_id = hit->get_detector_id();
      	parID_detID_ihit[std::make_tuple(track_id, det_id)] = ihit;

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
      }
    }
  }

  typedef std::tuple<int, int> ParRecoPair;
  std::map<ParRecoPair, int> parID_recID_nHit;

  typedef std::tuple<int, int> TrkIDNHit;
  std::map<int, TrkIDNHit> parID_bestRecID;
  std::map<int, TrkIDNHit> recID_bestParID;

  if(!_recEvent) {
  	LogInfo("!_recEvent");
  	return Fun4AllReturnCodes::ABORTRUN;
  }

  if(!_truth) {
  	LogInfo("!_truth");
  	return Fun4AllReturnCodes::ABORTRUN;
  }

	krecstat = _recEvent->getRecStatus();

	for(int itrack=0; itrack<_recEvent->getNTracks(); ++itrack){
		SRecTrack recTrack = _recEvent->getTrack(itrack);

		if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			cout << "--------- itrack: " << itrack << " ---------"<< endl;
		}

		// Fill map parID_recID => nHit
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

			// TODO better way to exclude hodo and prop hits?
			// this is try to exclude hodo and prop hits
			if(hit->get_detector_id() > 30) continue;

			int parID = hit->get_track_id();
			//LogInfo(parID);
      if(parID > 9999) continue;
			//LogInfo(parID);

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

		if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			LogInfo("");
			std::cout
			<< parID << " : " << recID
			<< " => " << nHit
			<< std::endl;
		}

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

	// Fill Reco Tree
	for(int itrack=0; itrack<_recEvent->getNTracks(); ++itrack){
		SRecTrack recTrack = _recEvent->getTrack(itrack);

		if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
			cout << "--------- itrack: " << itrack << " ---------"<< endl;
		}

		nhits[n_tracks] = recTrack.getNHits();
		charge[n_tracks] = recTrack.getCharge();
		TVector3 rec_vtx = recTrack.getTargetPos();
		vx[n_tracks]  = rec_vtx.X();
		vy[n_tracks]  = rec_vtx.Y();
		vz[n_tracks]  = rec_vtx.Z();
		TVector3 rec_mom = recTrack.getTargetMom();
		px[n_tracks]  = rec_mom.Px();
		py[n_tracks]  = rec_mom.Py();
		pz[n_tracks]  = rec_mom.Pz();
		pt[n_tracks]  = rec_mom.Pt();
		eta[n_tracks] = rec_mom.Eta();
		phi[n_tracks] = rec_mom.Phi();

		{
			double tx, ty, tz;
			recTrack.getMomentumSt1(tx, ty, tz);
			px_st1[n_tracks] = tx;
			py_st1[n_tracks] = ty;
			pz_st1[n_tracks] = tz;
			double x, y;
			recTrack.getPositionSt1(x, y);
			x_st1[n_tracks] = x;
			y_st1[n_tracks] = y;

		}

		int trackID = itrack;

		rec_id[n_tracks] = trackID;

		if(recID_bestParID.find(trackID)!=recID_bestParID.end()) {
			ntruhits[n_tracks] = std::get<1>(recID_bestParID[trackID]);
			int parID = std::get<0>(recID_bestParID[trackID]);

			PHG4Particle * par = _truth->GetParticle(parID);

			par_id[n_tracks] = parID;

			pid[n_tracks] = par->get_pid();

			int vtx_id =  par->get_vtx_id();
			PHG4VtxPoint* vtx = _truth->GetVtx(vtx_id);
			gvx[n_tracks] = vtx->get_x();
			gvy[n_tracks] = vtx->get_y();
			gvz[n_tracks] = vtx->get_z();

			TVector3 mom(par->get_px(), par->get_py(), par->get_pz());
			gpx[n_tracks] = par->get_px();
			gpy[n_tracks] = par->get_py();
			gpz[n_tracks] = par->get_pz();
			gpt[n_tracks] = mom.Pt();
			geta[n_tracks] = mom.Eta();
			gphi[n_tracks] = mom.Phi();

			// trackID + detID -> SQHit -> PHG4Hit -> momentum
			for(int det_id=7; det_id<=12; ++det_id) {
				auto iter = parID_detID_ihit.find(std::make_tuple(parID, det_id));
				if(iter != parID_detID_ihit.end()) {
					if(verbosity >= Fun4AllBase::VERBOSITY_A_LOT) {
						LogDebug("ihit: " << iter->second);
					}
					SQHit *hit = _hit_vector->at(iter->second);
					if(verbosity >= Fun4AllBase::VERBOSITY_A_LOT) {
						hit->identify();
					}
					if(hit and D1X_hits) {
						PHG4Hit* g4hit =  D1X_hits->findHit(hit->get_g4hit_id());
						if (g4hit) {
							if(verbosity >= 2) {
								g4hit->identify();
							}
							gx_st1[n_tracks]  = g4hit->get_x(0);
							gy_st1[n_tracks]  = g4hit->get_y(0);
							gz_st1[n_tracks]  = g4hit->get_z(0);

							gpx_st1[n_tracks] = g4hit->get_px(0)/1000.;
							gpy_st1[n_tracks] = g4hit->get_py(0)/1000.;
							gpz_st1[n_tracks] = g4hit->get_pz(0)/1000.;
							break;
						}
					}
				}
			}
  		gnhits[n_tracks] =
  				parID_nhits_dc[parID] +
					parID_nhits_hodo[parID] +
					parID_nhits_prop[parID];

  		gndc[n_tracks] = parID_nhits_dc[parID];
  		gnhodo[n_tracks] = parID_nhits_hodo[parID];
  		gnprop[n_tracks] = parID_nhits_prop[parID];
		}
		++n_tracks;
		if(n_tracks>=1000) break;
	}

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("parID_recID_nHit mapping finished");

  _tout_reco->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "Leaving TrkEval::RecoEval: " << _event << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}


int TrkEval::TruthEval(PHCompositeNode* topNode)
{
	if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
		std::cout << "Entering TrkEval::TruthEval: " << _event << std::endl;

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
    event_id    = _event_header->get_event_id();
    emu_trigger = _event_header->get_trigger();
    spill_id    = _event_header->get_spill_id();
    run_id      = _event_header->get_run_id();
  }

  PHG4HitContainer *D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D0X");
  if (!D1X_hits)
    D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D1X");

  if (!D1X_hits)
  {
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
        cout << Name() << " Could not locate g4 hit node " << "G4HIT_D0X or G4HIT_D1X" << endl;
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

      if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
      	LogInfo(hit->get_detector_id());
      	hit->identify();
      }

    	int hitID = hit->get_hit_id();
    	hit_id[n_hits]         = hitID;
    	hitID_ihit[hitID]      = ihit;
      drift_distance[n_hits] = hit->get_drift_distance();
      pos[n_hits]            = hit->get_pos();
      detector_z[n_hits]     = p_geomSvc->getPlanePosition(hit->get_detector_id());
      detector_id[n_hits]    = hit->get_detector_id();
      element_id[n_hits]     = hit->get_element_id();
      hodo_mask[n_hits]      = hit->is_hodo_mask();

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
      			p_geomSvc->getPlane(hit->get_detector_id()).uVec[0],
      			p_geomSvc->getPlane(hit->get_detector_id()).uVec[1],
      			p_geomSvc->getPlane(hit->get_detector_id()).uVec[2]
      	};

      	truth_pos[n_hits] =
//      			(truth_x[n_hits] - p_geomSvc->getPlane(hit->get_detector_id()).xc)*uVec[0] +
//      			(truth_y[n_hits] - p_geomSvc->getPlane(hit->get_detector_id()).yc)*uVec[1] +
//      			(truth_z[n_hits] - p_geomSvc->getPlane(hit->get_detector_id()).zc)*uVec[2];
  			(truth_x[n_hits])*uVec[0] +
  			(truth_y[n_hits])*uVec[1] +
  			(truth_z[n_hits]-p_geomSvc->getPlane(hit->get_detector_id()).zc)*uVec[2];

        if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
        	LogInfo("");
        	std::cout << truth_pos[n_hits] << " => { "
        			<< truth_x[n_hits] << ", "
        			<< truth_y[n_hits] << ", "
							<< truth_z[n_hits] << "} {"
							<< uVec[0] << ", "
							<< uVec[1] << ", "
							<< uVec[2] << "}"
							<< std::endl;
        }

      	//LogDebug("detector_id: " << detector_id[n_hits]);
      }
      ++n_hits;
      if(n_hits>=10000) break;
    }
  }

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("ghit eval finished");

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
        if(parID > 9999) continue;

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

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("parID_recID_nHit mapping finished");

  if(_truth) {
  	for(auto iter=_truth->GetPrimaryParticleRange().first;
  			iter!=_truth->GetPrimaryParticleRange().second;
  			++iter) {
  		PHG4Particle * par = iter->second;

  		pid[n_tracks] = par->get_pid();

  		int vtx_id =  par->get_vtx_id();
  		PHG4VtxPoint* vtx = _truth->GetVtx(vtx_id);
  		gvx[n_tracks] = vtx->get_x();
  		gvy[n_tracks] = vtx->get_y();
  		gvz[n_tracks] = vtx->get_z();

  		TVector3 mom(par->get_px(), par->get_py(), par->get_pz());
  		gpx[n_tracks] = par->get_px();
  		gpy[n_tracks] = par->get_py();
  		gpz[n_tracks] = par->get_pz();
  		gpt[n_tracks] = mom.Pt();
  		geta[n_tracks] = mom.Eta();
  		gphi[n_tracks] = mom.Phi();

  		int parID = par->get_track_id();

			par_id[n_tracks] = parID;

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
					if(hit and D1X_hits) {
  					PHG4Hit* g4hit =  D1X_hits->findHit(hit->get_g4hit_id());
  					if (g4hit) {
  						if(verbosity >= 2) {
  							g4hit->identify();
  						}
							gx_st1[n_tracks]  = g4hit->get_x(0);
							gy_st1[n_tracks]  = g4hit->get_y(0);
							gz_st1[n_tracks]  = g4hit->get_z(0);

							gpx_st1[n_tracks] = g4hit->get_px(0)/1000.;
							gpy_st1[n_tracks] = g4hit->get_py(0)/1000.;
							gpz_st1[n_tracks] = g4hit->get_pz(0)/1000.;
							break;
  					}
  				}
  			}
  		}

  		gnhits[n_tracks] =
  				parID_nhits_dc[parID] +
					parID_nhits_hodo[parID] +
					parID_nhits_prop[parID];

  		gndc[n_tracks] = parID_nhits_dc[parID];
  		gnhodo[n_tracks] = parID_nhits_hodo[parID];
  		gnprop[n_tracks] = parID_nhits_prop[parID];

  		for(auto detid_elmid : parID_detid_elmid[parID]) {
  			int detid = detid_elmid.first;
  			int elmid = detid_elmid.second;
  			if(detid>=55) {
  				LogWarning("detid>=55");
  				continue;
  			}
  			gelmid[n_tracks][detid] = elmid;
  		}

  		if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("particle eval finished");

  		ntruhits[n_tracks] = 0;
  		if(parID_bestRecID.find(parID)!=parID_bestRecID.end()) {
  			ntruhits[n_tracks] = std::get<1>(parID_bestRecID[parID]);
  			int recID = std::get<0>(parID_bestRecID[parID]);
  			rec_id[n_tracks] = parID;
  			SRecTrack recTrack = _recEvent->getTrack(recID);
  			nhits[n_tracks]  = recTrack.getNHits();
  			charge[n_tracks] = recTrack.getCharge();
  			TVector3 rec_vtx = recTrack.getTargetPos();
  			vx[n_tracks]  = rec_vtx.X();
  			vy[n_tracks]  = rec_vtx.Y();
  			vz[n_tracks]  = rec_vtx.Z();
  			TVector3 rec_mom = recTrack.getTargetMom();
  			px[n_tracks]  = rec_mom.Px();
  			py[n_tracks]  = rec_mom.Py();
  			pz[n_tracks]  = rec_mom.Pz();
  			pt[n_tracks]  = rec_mom.Pt();
  			eta[n_tracks] = rec_mom.Eta();
  			phi[n_tracks] = rec_mom.Phi();

  			{
					double tx, ty, tz;
					recTrack.getMomentumSt1(tx, ty, tz);
					px_st1[n_tracks] = tx;
					py_st1[n_tracks] = ty;
					pz_st1[n_tracks] = tz;
					double x, y;
					recTrack.getPositionSt1(x, y);
					x_st1[n_tracks] = x;
					y_st1[n_tracks] = y;

  			}

//  			recTrack.getMomentumSt1(
//  					&px_st1[n_tracks],
//  					&py_st1[n_tracks],
//  					&pz_st1[n_tracks]
//						);
  		}

  		if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("single reco. eval finished");

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

			if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("dimu reco. eval finished");

  		++n_tracks;
  		if(n_tracks>=1000) break;
  	}
  }

  _tout_truth->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "Leaving TrkEval::TruthEval: " << _event << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "TrkEval::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout_truth->Write();
  _tout_reco->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int TrkEval::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout_truth = new TTree("Truth", "Truth Eval");
  _tout_truth->Branch("runID",         &run_id,          "runID/I");
  _tout_truth->Branch("spillID",       &spill_id,        "spillID/I");
  _tout_truth->Branch("liveProton",    &target_pos,      "liveProton/F");
  _tout_truth->Branch("eventID",       &event_id,        "eventID/I");
  _tout_truth->Branch("emu_trigger",   &emu_trigger,      "emu_trigger/s");
  _tout_truth->Branch("krecstat",      &krecstat,        "krecstat/I");

  _tout_truth->Branch("nHits",         &n_hits,          "nHits/I");
  _tout_truth->Branch("hitID",         hit_id,           "hitID[nHits]/I");
  _tout_truth->Branch("detectorID",    detector_id,      "detectorID[nHits]/I");
  _tout_truth->Branch("elementID",     element_id,       "elementID[nHits]/I");
  _tout_truth->Branch("hodo_mask",     hodo_mask,        "hodo_mask[nHits]/I");
  _tout_truth->Branch("detectorZ",     detector_z,       "detectorZ[nHits]/F");
  _tout_truth->Branch("truth_x",       truth_x,          "truth_x[nHits]/F");
  _tout_truth->Branch("truth_y",       truth_y,          "truth_y[nHits]/F");
  _tout_truth->Branch("truth_z",       truth_z,          "truth_z[nHits]/F");
  _tout_truth->Branch("truth_pos",     truth_pos,        "truth_pos[nHits]/F");
  _tout_truth->Branch("driftDistance", drift_distance,   "driftDistance[nHits]/F");
  _tout_truth->Branch("pos",           pos,              "pos[nHits]/F");

  _tout_truth->Branch("n_tracks",      &n_tracks,           "n_tracks/I");
  _tout_truth->Branch("par_id",        par_id,              "par_id[n_tracks]/I");
  _tout_truth->Branch("rec_id",        rec_id,              "rec_id[n_tracks]/I");
  _tout_truth->Branch("pid",           pid,                 "pid[n_tracks]/I");
  _tout_truth->Branch("gvx",           gvx,                 "gvx[n_tracks]/F");
  _tout_truth->Branch("gvy",           gvy,                 "gvy[n_tracks]/F");
  _tout_truth->Branch("gvz",           gvz,                 "gvz[n_tracks]/F");
  _tout_truth->Branch("gpx",           gpx,                 "gpx[n_tracks]/F");
  _tout_truth->Branch("gpy",           gpy,                 "gpy[n_tracks]/F");
  _tout_truth->Branch("gpz",           gpz,                 "gpz[n_tracks]/F");
  _tout_truth->Branch("gpt",           gpt,                 "gpt[n_tracks]/F");
  _tout_truth->Branch("geta",          geta,                "geta[n_tracks]/F");
  _tout_truth->Branch("gphi",          gphi,                "gphi[n_tracks]/F");
  _tout_truth->Branch("gx_st1",        gx_st1,              "gx_st1[n_tracks]/F");
  _tout_truth->Branch("gy_st1",        gy_st1,              "gy_st1[n_tracks]/F");
  _tout_truth->Branch("gz_st1",        gz_st1,              "gz_st1[n_tracks]/F");
  _tout_truth->Branch("gpx_st1",       gpx_st1,             "gpx_st1[n_tracks]/F");
  _tout_truth->Branch("gpy_st1",       gpy_st1,             "gpy_st1[n_tracks]/F");
  _tout_truth->Branch("gpz_st1",       gpz_st1,             "gpz_st1[n_tracks]/F");
  _tout_truth->Branch("gnhits",        gnhits,              "gnhits[n_tracks]/I");
  _tout_truth->Branch("gndc",          gndc,                "gndc[n_tracks]/I");
  _tout_truth->Branch("gnhodo",        gnhodo,              "gnhodo[n_tracks]/I");
  _tout_truth->Branch("gnprop",        gnprop,              "gnprop[n_tracks]/I");
  _tout_truth->Branch("gelmid",        gelmid,              "gelmid[n_tracks][54]/I");

  _tout_truth->Branch("ntruhits",      ntruhits,            "ntruhits[n_tracks]/I");
  _tout_truth->Branch("nhits",         nhits,               "nhits[n_tracks]/I");
  _tout_truth->Branch("charge",        charge,              "charge[n_tracks]/I");
  _tout_truth->Branch("vx",            vx,                  "vx[n_tracks]/F");
  _tout_truth->Branch("vy",            vy,                  "vy[n_tracks]/F");
  _tout_truth->Branch("vz",            vz,                  "vz[n_tracks]/F");
  _tout_truth->Branch("px",            px,                  "px[n_tracks]/F");
  _tout_truth->Branch("py",            py,                  "py[n_tracks]/F");
  _tout_truth->Branch("pz",            pz,                  "pz[n_tracks]/F");
  _tout_truth->Branch("pt",            pt,                  "pt[n_tracks]/F");
  _tout_truth->Branch("eta",           eta,                 "eta[n_tracks]/F");
  _tout_truth->Branch("phi",           phi,                 "phi[n_tracks]/F");
  _tout_truth->Branch("x_st1",         x_st1,               "x_st1[n_tracks]/F");
  _tout_truth->Branch("y_st1",         y_st1,               "y_st1[n_tracks]/F");
  _tout_truth->Branch("px_st1",        px_st1,              "px_st1[n_tracks]/F");
  _tout_truth->Branch("py_st1",        py_st1,              "py_st1[n_tracks]/F");
  _tout_truth->Branch("pz_st1",        pz_st1,              "pz_st1[n_tracks]/F");

  _tout_truth->Branch("gndimu",        &gndimu,              "gndimu/I");
  _tout_truth->Branch("dimu_gpx",      dimu_gpx,             "dimu_gpx[gndimu]/F");
  _tout_truth->Branch("dimu_gpy",      dimu_gpy,             "dimu_gpy[gndimu]/F");
  _tout_truth->Branch("dimu_gpz",      dimu_gpz,             "dimu_gpz[gndimu]/F");
  _tout_truth->Branch("dimu_gpt",      dimu_gpt,             "dimu_gpt[gndimu]/F");
  _tout_truth->Branch("dimu_gmass",    dimu_gmass,           "dimu_gmass[gndimu]/F");
  _tout_truth->Branch("dimu_geta",     dimu_geta,            "dimu_geta[gndimu]/F");
  _tout_truth->Branch("dimu_gphi",     dimu_gphi,            "dimu_gphi[gndimu]/F");

  _tout_truth->Branch("dimu_nrec",     dimu_nrec,            "dimu_nrec[gndimu]/I");
  _tout_truth->Branch("dimu_px",       dimu_px,              "dimu_px[gndimu]/F");
  _tout_truth->Branch("dimu_py",       dimu_py,              "dimu_py[gndimu]/F");
  _tout_truth->Branch("dimu_pz",       dimu_pz,              "dimu_pz[gndimu]/F");
  _tout_truth->Branch("dimu_pt",       dimu_pt,              "dimu_pt[gndimu]/F");
  _tout_truth->Branch("dimu_mass",     dimu_mass,            "dimu_mass[gndimu]/F");
  _tout_truth->Branch("dimu_eta",      dimu_eta,             "dimu_eta[gndimu]/F");
  _tout_truth->Branch("dimu_phi",      dimu_phi,             "dimu_phi[gndimu]/F");

  _tout_reco = new TTree("Reco", "Reco Eval");

  _tout_reco->Branch("krecstat",      &krecstat,           "krecstat/I");
  _tout_reco->Branch("n_tracks",      &n_tracks,           "n_tracks/I");
  _tout_reco->Branch("par_id",        par_id,              "par_id[n_tracks]/I");
  _tout_reco->Branch("rec_id",        rec_id,              "rec_id[n_tracks]/I");
  _tout_reco->Branch("pid",           pid,                 "pid[n_tracks]/I");
  _tout_reco->Branch("gvx",           gvx,                 "gvx[n_tracks]/F");
  _tout_reco->Branch("gvy",           gvy,                 "gvy[n_tracks]/F");
  _tout_reco->Branch("gvz",           gvz,                 "gvz[n_tracks]/F");
  _tout_reco->Branch("gpx",           gpx,                 "gpx[n_tracks]/F");
  _tout_reco->Branch("gpy",           gpy,                 "gpy[n_tracks]/F");
  _tout_reco->Branch("gpz",           gpz,                 "gpz[n_tracks]/F");
  _tout_reco->Branch("gpt",           gpt,                 "gpt[n_tracks]/F");
  _tout_reco->Branch("geta",          geta,                "geta[n_tracks]/F");
  _tout_reco->Branch("gphi",          gphi,                "gphi[n_tracks]/F");
  _tout_reco->Branch("gx_st1",        gx_st1,              "gx_st1[n_tracks]/F");
  _tout_reco->Branch("gy_st1",        gy_st1,              "gy_st1[n_tracks]/F");
  _tout_reco->Branch("gz_st1",        gz_st1,              "gz_st1[n_tracks]/F");
  _tout_reco->Branch("gpx_st1",       gpx_st1,             "gpx_st1[n_tracks]/F");
  _tout_reco->Branch("gpy_st1",       gpy_st1,             "gpy_st1[n_tracks]/F");
  _tout_reco->Branch("gpz_st1",       gpz_st1,             "gpz_st1[n_tracks]/F");
  _tout_reco->Branch("gnhits",        gnhits,              "gnhits[n_tracks]/I");
  _tout_reco->Branch("gndc",          gndc,                "gndc[n_tracks]/I");
  _tout_reco->Branch("gnhodo",        gnhodo,              "gnhodo[n_tracks]/I");
  _tout_reco->Branch("gnprop",        gnprop,              "gnprop[n_tracks]/I");
  //_tout_reco->Branch("gelmid",        gelmid,              "gelmid[n_tracks][54]/I");

  _tout_reco->Branch("ntruhits",      ntruhits,            "ntruhits[n_tracks]/I");
  _tout_reco->Branch("nhits",         nhits,               "nhits[n_tracks]/I");
  _tout_reco->Branch("charge",        charge,              "charge[n_tracks]/I");
  _tout_reco->Branch("vx",            vx,                  "vx[n_tracks]/F");
  _tout_reco->Branch("vy",            vy,                  "vy[n_tracks]/F");
  _tout_reco->Branch("vz",            vz,                  "vz[n_tracks]/F");
  _tout_reco->Branch("px",            px,                  "px[n_tracks]/F");
  _tout_reco->Branch("py",            py,                  "py[n_tracks]/F");
  _tout_reco->Branch("pz",            pz,                  "pz[n_tracks]/F");
  _tout_reco->Branch("pt",            pt,                  "pt[n_tracks]/F");
  _tout_reco->Branch("eta",           eta,                 "eta[n_tracks]/F");
  _tout_reco->Branch("phi",           phi,                 "phi[n_tracks]/F");
  _tout_reco->Branch("x_st1",         x_st1,               "x_st1[n_tracks]/F");
  _tout_reco->Branch("y_st1",         y_st1,               "y_st1[n_tracks]/F");
  _tout_reco->Branch("px_st1",        px_st1,              "px_st1[n_tracks]/F");
  _tout_reco->Branch("py_st1",        py_st1,              "py_st1[n_tracks]/F");
  _tout_reco->Branch("pz_st1",        pz_st1,              "pz_st1[n_tracks]/F");

  return 0;
}

int TrkEval::ResetEvalVars() {
  run_id = std::numeric_limits<int>::max();
  spill_id = std::numeric_limits<int>::max();
  target_pos = std::numeric_limits<float>::max();
  event_id = std::numeric_limits<int>::max();
  emu_trigger = 0;
  krecstat = std::numeric_limits<int>::max();

  n_hits = 0;
  for(int i=0; i<10000; ++i) {
    detector_id[i]    = std::numeric_limits<short>::max();
    element_id[i]     = std::numeric_limits<short>::max();
    hodo_mask[i]      = std::numeric_limits<short>::max();
    drift_distance[i] = std::numeric_limits<float>::max();
    pos[i]            = std::numeric_limits<float>::max();
    detector_z[i]     = std::numeric_limits<float>::max();

    truth_x[i]       = std::numeric_limits<float>::max();
    truth_y[i]       = std::numeric_limits<float>::max();
    truth_z[i]       = std::numeric_limits<float>::max();
    truth_pos[i]     = std::numeric_limits<float>::max();
  }

  n_tracks = 0;
  for(int i=0; i<1000; ++i) {
    rec_id[i]     = std::numeric_limits<int>::max();
    par_id[i]     = std::numeric_limits<int>::max();
    pid[i]        = std::numeric_limits<int>::max();
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
    gx_st1[i]     = std::numeric_limits<float>::max();
    gy_st1[i]     = std::numeric_limits<float>::max();
    gz_st1[i]     = std::numeric_limits<float>::max();
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
    nhits[i]      = std::numeric_limits<int>::max();
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
    x_st1[i]     = std::numeric_limits<float>::max();
    y_st1[i]     = std::numeric_limits<float>::max();
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
    //return Fun4AllReturnCodes::ABORTEVENT;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}







