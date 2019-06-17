/**
 * \class PatternDBGen
 * \brief General purposed evaluation module
 * \author Haiwang Yu, yuhw@nmsu.edu
 *
 * Created: 08-27-2018
 */


#include "PatternDBGen.h"

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

#define LogDebug(exp)   std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogError(exp)   std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl
#define LogWarning(exp)     std::cout<<"WARNING: "<<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

PatternDBGen::PatternDBGen(const std::string& name) :
SubsysReco(name),
_hit_container_type("Vector"),
_event(0),
_event_header(nullptr),
_hit_map(nullptr),
_hit_vector(nullptr),
_out_name("pattern_db.root")
{
}

int PatternDBGen::Init(PHCompositeNode* topNode) {
  return Fun4AllReturnCodes::EVENT_OK;
}

int PatternDBGen::InitRun(PHCompositeNode* topNode) {

  ResetEvalVars();
  InitEvalTree();

  p_geomSvc = GeomSvc::instance();

  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  return Fun4AllReturnCodes::EVENT_OK;
}

int PatternDBGen::process_event(PHCompositeNode* topNode) {
  int ret = Fun4AllReturnCodes::ABORTRUN;

  ret = TruthEval(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ++_event;

  return ret;
}

int PatternDBGen::TruthEval(PHCompositeNode* topNode)
{
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "Entering PatternDBGen::TruthEval: " << _event << std::endl;

  PHG4HitContainer *D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D0X");
  if (!D1X_hits)
    D1X_hits = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D1X");

  if (!D1X_hits)
  {
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
        cout << Name() << " Could not locate g4 hit node " << "G4HIT_D0X or G4HIT_D1X" << endl;
  }

  ResetEvalVars();

  std::map<int, int> parID_nhits_dc;
  std::map<int, int> parID_nhits_hodo;
  std::map<int, int> parID_nhits_prop;

  std::map<int, std::map<int, int> > parID_detid_elmid;

  typedef std::tuple<int, int> ParDetPair;
  std::map<ParDetPair, int> parID_detID_ihit;

  std::map<int, int> hitID_ihit;

  if(_hit_vector) {
    for(int ihit=0; ihit<_hit_vector->size(); ++ihit) {
      SQHit *hit = _hit_vector->at(ihit);

      if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
        LogInfo(hit->get_detector_id());
        hit->identify();
      }

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
      }
    }
  }

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("ghit eval finished");

  n_tracks = 0;
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

      // Get truth track par at station 1
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

      ++n_tracks;
      if(n_tracks>=1000) break;
    }
  }

  _tout_truth->Fill();

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "Leaving PatternDBGen::TruthEval: " << _event << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}

int PatternDBGen::End(PHCompositeNode* topNode) {
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
    std::cout << "PatternDBGen::End" << std::endl;

  PHTFileServer::get().cd(_out_name.c_str());
  _tout_truth->Write();

  return Fun4AllReturnCodes::EVENT_OK;
}

int PatternDBGen::InitEvalTree() {
  PHTFileServer::get().open(_out_name.c_str(), "RECREATE");

  _tout_truth = new TTree("T", "Truth Eval");
//  _tout_truth->Branch("runID",         &run_id,          "runID/I");
//  _tout_truth->Branch("spillID",       &spill_id,        "spillID/I");
//  _tout_truth->Branch("liveProton",    &target_pos,      "liveProton/F");
//  _tout_truth->Branch("eventID",       &event_id,        "eventID/I");
//
  _tout_truth->Branch("n_tracks",      &n_tracks,           "n_tracks/I");
  _tout_truth->Branch("pid",           pid,                 "pid[n_tracks]/I");
//  _tout_truth->Branch("gvx",           gvx,                 "gvx[n_tracks]/F");
//  _tout_truth->Branch("gvy",           gvy,                 "gvy[n_tracks]/F");
//  _tout_truth->Branch("gvz",           gvz,                 "gvz[n_tracks]/F");
//  _tout_truth->Branch("gpx",           gpx,                 "gpx[n_tracks]/F");
//  _tout_truth->Branch("gpy",           gpy,                 "gpy[n_tracks]/F");
//  _tout_truth->Branch("gpz",           gpz,                 "gpz[n_tracks]/F");
//  _tout_truth->Branch("gpt",           gpt,                 "gpt[n_tracks]/F");
//  _tout_truth->Branch("geta",          geta,                "geta[n_tracks]/F");
//  _tout_truth->Branch("gphi",          gphi,                "gphi[n_tracks]/F");
//  _tout_truth->Branch("gx_st1",        gx_st1,              "gx_st1[n_tracks]/F");
//  _tout_truth->Branch("gy_st1",        gy_st1,              "gy_st1[n_tracks]/F");
//  _tout_truth->Branch("gz_st1",        gz_st1,              "gz_st1[n_tracks]/F");
//  _tout_truth->Branch("gpx_st1",       gpx_st1,             "gpx_st1[n_tracks]/F");
//  _tout_truth->Branch("gpy_st1",       gpy_st1,             "gpy_st1[n_tracks]/F");
//  _tout_truth->Branch("gpz_st1",       gpz_st1,             "gpz_st1[n_tracks]/F");
//  _tout_truth->Branch("gnhits",        gnhits,              "gnhits[n_tracks]/I");
  _tout_truth->Branch("gndc",          gndc,                "gndc[n_tracks]/I");
//  _tout_truth->Branch("gnhodo",        gnhodo,              "gnhodo[n_tracks]/I");
//  _tout_truth->Branch("gnprop",        gnprop,              "gnprop[n_tracks]/I");
  _tout_truth->Branch("gelmid",        gelmid,              "gelmid[n_tracks][54]/I");

  return 0;
}

int PatternDBGen::ResetEvalVars() {
  run_id = std::numeric_limits<int>::max();
  spill_id = std::numeric_limits<int>::max();
  target_pos = std::numeric_limits<float>::max();
  event_id = std::numeric_limits<int>::max();

  n_tracks = 0;
  for(int i=0; i<1000; ++i) {
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
  }

  return 0;
}

int PatternDBGen::GetNodes(PHCompositeNode* topNode) {

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

  return Fun4AllReturnCodes::EVENT_OK;
}







