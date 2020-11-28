#include <iomanip>
#include <list>
#include <phhepmc/PHHepMCGenEventMap.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4VtxPoint.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQTrack_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQTrackVector_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQMCHit_v1.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <ktracker/SRecEvent.h>
#include <GlobalConsts.h>
#include "TruthNodeMaker.h"
using namespace std;

TruthNodeMaker::TruthNodeMaker()
  : SubsysReco("TruthNodeMaker")
  , genevtmap(nullptr)
  , g4true(nullptr)
  , m_vec_hit(nullptr)
  , m_rec_evt(nullptr)
  , m_vec_rec_trk(nullptr)
  , m_evt(nullptr)
  , m_mcevt(nullptr)
  , m_vec_trk(nullptr)
  , m_vec_dim(nullptr)
  , m_legacy_rec_container(true)
  , m_do_evt_header(true)
  , m_do_truthtrk_tagging(true)
  , m_matching_threshold(0.75)
{
  for(int i = 0; i <= nChamberPlanes; ++i) {
    m_g4hc[i] = nullptr;
  }
}

TruthNodeMaker::~TruthNodeMaker()
{
  if (! m_evt    ) delete m_evt;
  if (! m_mcevt  ) delete m_mcevt;
  if (! m_vec_trk) delete m_vec_trk;
  if (! m_vec_dim) delete m_vec_dim;
}

int TruthNodeMaker::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int TruthNodeMaker::InitRun(PHCompositeNode* topNode)
{
  int ret = GetNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  ret = MakeNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  return Fun4AllReturnCodes::EVENT_OK;
}

int TruthNodeMaker::process_event(PHCompositeNode* topNode)
{
  /// Extract the hard process.
  if(m_do_evt_header) {
    if (genevtmap->size() != 1) {
      cout << "TruthNodeMaker::process_event(): size != 1 unexpectedly." << endl;
    }

    for (PHHepMCGenEventMap::Iter iter = genevtmap->begin(); iter != genevtmap->end(); ++iter) {
      PHHepMCGenEvent *genevt = iter->second;
      if (! genevt) {
        cout << "No PHHepMCGenEvent object." << endl;
        //return Fun4AllReturnCodes::ABORTEVENT;
      }
      HepMC::GenEvent *evt = genevt->getEvent();
      if (! evt) {
        cout << "No HepMC::GenEvent object." << endl;
        //return Fun4AllReturnCodes::ABORTEVENT;
      }
      m_evt->set_run_id  (0);
      m_evt->set_spill_id(0);
      m_evt->set_event_id(evt->event_number());
      m_mcevt->set_process_id(evt->signal_process_id());

      //HepMC::GenVertex* vtx = evt->signal_process_vertex(); // Return 0 as of 2019-11-19.
      HepMC::GenEvent::particle_const_iterator it = evt->particles_begin();
      it++; // Skip the 1st beam particle.
      for (int iii = 0; iii < 4; iii++) {
        it++;
        const HepMC::GenParticle* par = *it;
        const HepMC::FourVector * mom = &par->momentum();
        TLorentzVector lvec;
        lvec.SetPxPyPzE(mom->px(), mom->py(), mom->pz(), mom->e());
        m_mcevt->set_particle_id(iii, par->pdg_id());
        m_mcevt->set_particle_momentum(iii, lvec);
      }
    }
  }

  /// Extract the true-track info
  m_vec_trk->clear();
  map<int, int> trkid_idx;
  vector<int> vec_vtx_id;
  for (auto it = g4true->GetPrimaryParticleRange().first; it != g4true->GetPrimaryParticleRange().second; ++it) {
    PHG4Particle* par = it->second;
    int pid = par->get_pid();
    if (abs(pid) != 13) continue; // not muon
    int trk_id = par->get_track_id();
    int vtx_id = par->get_vtx_id();
    PHG4VtxPoint* vtx = g4true->GetVtx(vtx_id);

    SQTrack_v1 trk;
    trk.set_track_id(trk_id);
    trk.set_charge(pid < 0 ? +1: -1); // -13 = mu+
    trk.set_pos_vtx(TVector3      (vtx->get_x (), vtx->get_y (), vtx->get_z ()));
    trk.set_mom_vtx(TLorentzVector(par->get_px(), par->get_py(), par->get_pz(), par->get_e()));

    trkid_idx[trk_id] = m_vec_trk->size();
    m_vec_trk->push_back(&trk);
    vec_vtx_id.push_back(vtx_id);
  }

  /// Construct the dimuon info
  m_vec_dim->clear();
  int id_dim = 0; // to be incremented
  unsigned int n_trk = m_vec_trk->size();
  for (unsigned int i1 = 0; i1 < n_trk; i1++) {
    SQTrack* trk1 = m_vec_trk->at(i1);
    if (trk1->get_charge() <= 0) continue;
    for (unsigned int i2 = 0; i2 < n_trk; i2++) {
      SQTrack* trk2 = m_vec_trk->at(i2);
      if (trk2->get_charge() >= 0) continue;
      if (vec_vtx_id[i1] != vec_vtx_id[i2]) continue;

      SQDimuon_v1 dim;
      dim.set_dimuon_id(++id_dim);
      //dim.set_pdg_id(par_dim->pdg_id()); // PDG ID is not accessible via PHG4Particle
      dim.set_pos         (trk1->get_pos_vtx());
      dim.set_mom         (trk1->get_mom_vtx() + trk2->get_mom_vtx());
      dim.set_mom_pos     (trk1->get_mom_vtx());
      dim.set_mom_neg     (trk2->get_mom_vtx());
      dim.set_track_id_pos(trk1->get_track_id());
      dim.set_track_id_neg(trk2->get_track_id());
      m_vec_dim->push_back(&dim);
    }
  }

  /// link digi hits to truth tracks
  map<int, vector<SQHit*> > trkid_hitvec;
  int n_digihits = m_vec_hit->size();
  for(int i = 0; i < n_digihits; ++i) {
    SQHit* hit = m_vec_hit->at(i);

    int detid = hit->get_detector_id();
    if(detid > nChamberPlanes || (detid >= 7 && detid <= 12)) continue;

    int trkid = hit->get_track_id();
    if(trkid_idx.find(trkid) == trkid_idx.end()) continue;

    if(trkid_hitvec.find(trkid) == trkid_hitvec.end()) {
      trkid_hitvec[trkid] = vector<SQHit*>();
    }
    trkid_hitvec[trkid].push_back(hit);
  }

  /// Update truth track pos/mom at station-1/3
  for(unsigned int i = 0; i < n_trk; ++i) {
    SQTrack* trk = m_vec_trk->at(i);

    int trkid = trk->get_track_id();
    if(trkid_hitvec.find(trkid) == trkid_hitvec.end()) {
      //this track does not have hits in detector
      continue;
    }
    trk->set_num_hits(trkid_hitvec[trkid].size());

    TVector3 pos;
    TLorentzVector mom;
    int detIDs_st1[6] = {3, 4, 2, 5, 1, 6};
    if(FindHitAtStation(detIDs_st1, trkid, trkid_hitvec[trkid], pos, mom)) {
      trk->set_pos_st1(pos);
      trk->set_mom_st1(mom);
    }

    int detIDs_st3p[6] = {21, 22, 20, 23, 19, 24};
    int detIDs_st3m[6] = {27, 28, 26, 29, 25, 30};
    if(FindHitAtStation(detIDs_st3p, trkid, trkid_hitvec[trkid], pos, mom) ||
       FindHitAtStation(detIDs_st3m, trkid, trkid_hitvec[trkid], pos, mom)) {
      trk->set_pos_st3(pos);
      trk->set_mom_st3(mom);
    }
  }

  /// optionally, link the rec track to truth track
  if(!m_do_truthtrk_tagging) return Fun4AllReturnCodes::EVENT_OK;

  //// extract the list of hitid for each rec track
  int n_rtrks = 0;
  map<int, vector<int> > rtrkid_hitidvec;
  if(m_legacy_rec_container) {
    n_rtrks = m_rec_evt->getNTracks();
    for(int i = 0; i < n_rtrks; i++) {
      SRecTrack& recTrack = m_rec_evt->getTrack(i);
      rtrkid_hitidvec[i] = vector<int>();
      
      int n_rhits = recTrack.getNHits();
      for(int j = 0; j < n_rhits; ++j) { 
        rtrkid_hitidvec[i].push_back(recTrack.getHitIndex(j));
      }

      sort(rtrkid_hitidvec[i].begin(), rtrkid_hitidvec[i].end());
    }
  } else {
    n_rtrks = m_vec_rec_trk->size();
    for(int i = 0; i < n_rtrks; ++i) {
      SRecTrack* recTrack = dynamic_cast<SRecTrack*>(m_vec_rec_trk->at(i));
      rtrkid_hitidvec[i] = vector<int>();
      
      int n_rhits = recTrack->getNHits();
      for(int j = 0; j < n_rhits; ++j) { 
        rtrkid_hitidvec[i].push_back(recTrack->getHitIndex(j));
      }

      sort(rtrkid_hitidvec[i].begin(), rtrkid_hitidvec[i].end());
    }
  }
  if(n_rtrks <= 0) return Fun4AllReturnCodes::EVENT_OK;

  //// extract the list of hitid for each true track
  map<int, vector<int> > ttrkid_hitidvec;
  for(auto it = trkid_hitvec.begin(); it != trkid_hitvec.end(); ++it) {
    ttrkid_hitidvec[it->first] = vector<int>();
    for(auto jt = it->second.begin(); jt != it->second.end(); ++jt) {
      ttrkid_hitidvec[it->first].push_back((*jt)->get_hit_id());
    }
    sort(ttrkid_hitidvec[it->first].begin(), ttrkid_hitidvec[it->first].end());
  }

  //// for each truth track, find the best matching rec track and update rec track id
  for(unsigned int i = 0; i < n_trk; ++i) {
    SQTrack* trk = m_vec_trk->at(i);
    int ttrkid = trk->get_track_id();

    int rtrkid = -1;
    unsigned int n_match = 0;
    for(auto it = rtrkid_hitidvec.begin(); it != rtrkid_hitidvec.end(); ++it) {
      int n_match_new = FindCommonHitIDs(ttrkid_hitidvec[ttrkid], it->second);
      if(n_match < n_match_new) {
        n_match = n_match_new;
        rtrkid = it->first;
      }
    }

    if(rtrkid >= 0 && double(n_match)/double(ttrkid_hitidvec[ttrkid].size()) > m_matching_threshold) {
      trk->set_rec_track_id(rtrkid);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int TruthNodeMaker::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int TruthNodeMaker::GetNodes(PHCompositeNode* topNode)
{
  g4true = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if(!g4true) {
    cerr << Name() << ": failed locating G4TruthInfo, abort" << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  m_vec_hit = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if(!m_vec_hit) {
    cerr << Name() << ": failed locating the digitized hit vector, abort" << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  genevtmap = findNode::getClass<PHHepMCGenEventMap>(topNode, "PHHepMCGenEventMap");
  if(!genevtmap) {
    m_do_evt_header = false;
    cout << Name() << ": failed locating HepMCGenEvent, event process info will be missing" << endl;
  }

  GeomSvc* p_geomSvc = GeomSvc::instance();
  for(int i = 1; i <= nChamberPlanes; ++i)
  {
    string g4hitNodeName = "G4HIT_" + p_geomSvc->getDetectorName(i);
    m_g4hc[i] = findNode::getClass<PHG4HitContainer>(topNode, g4hitNodeName);
  }

  if(m_legacy_rec_container) {
    m_rec_evt = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
    if(!m_rec_evt) {
      m_do_truthtrk_tagging = false;
      cout << Name() << ": failed locating rec event, no truth track tagging." << endl;
    }
  } else {
    m_vec_rec_trk = findNode::getClass<SQTrackVector>(topNode, "SQRecTrackVector");
    if(!m_vec_rec_trk) {
      m_do_truthtrk_tagging = false;
      cout << Name() << ": failed locating rec track vector, no truth track tagging." << endl;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int TruthNodeMaker::MakeNodes(PHCompositeNode* topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* node_dst = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (! node_dst) {
    cout << "No DST node!?" << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  m_evt = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! m_evt) {
    m_evt = new SQEvent_v1();
    node_dst->addNode(new PHIODataNode<PHObject>(m_evt, "SQEvent", "PHObject"));
  }

  m_mcevt = findNode::getClass<SQMCEvent>(topNode, "SQMCEvent");
  if (! m_mcevt) {
    m_mcevt = new SQMCEvent_v1();
    node_dst->addNode(new PHIODataNode<PHObject>(m_mcevt, "SQMCEvent", "PHObject"));
  }

  m_vec_trk = findNode::getClass<SQTrackVector>(topNode, "SQTruthTrackVector");
  if (! m_vec_trk) {
    m_vec_trk = new SQTrackVector_v1();
    node_dst->addNode(new PHIODataNode<PHObject>(m_vec_trk, "SQTruthTrackVector", "PHObject"));
  }

  m_vec_dim = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if (! m_vec_dim) {
    m_vec_dim = new SQDimuonVector_v1();
    node_dst->addNode(new PHIODataNode<PHObject>(m_vec_dim, "SQTruthDimuonVector", "PHObject"));
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

bool TruthNodeMaker::FindHitAtStation(int target_detIDs[], int trkid, const vector<SQHit*>& hitvec, TVector3& pos, TLorentzVector& mom)
{
  for(int i = 0; i < 6; ++i) {
    //We try to find the wanted hit from SQHitVector first
    for(unsigned int j = 0; j < hitvec.size(); ++j) {
      if(hitvec[j]->get_detector_id() == target_detIDs[i]) {
        pos.SetXYZ(hitvec[j]->get_truth_x(), hitvec[j]->get_truth_y(), hitvec[j]->get_truth_z());
        mom.SetXYZM(hitvec[j]->get_truth_px(), hitvec[j]->get_truth_py(), hitvec[j]->get_truth_pz(), M_MU);
        return true;
      }
    }

    //if failed, we resort to the default solution of G4HIT list
    if(!m_g4hc[target_detIDs[i]]) continue;
    PHG4HitContainer::ConstRange range = m_g4hc[target_detIDs[i]]->getHits();
    for(PHG4HitContainer::ConstIterator it = range.first; it != range.second; it++) {
      PHG4Hit* hit = it->second;
      if(hit->get_trkid() == trkid) {
        pos.SetXYZ (hit->get_x(0)     , hit->get_y(0)     , hit->get_z(0)       );
        mom.SetXYZM(hit->get_px(0),     hit->get_py(0),     hit->get_pz(0), M_MU);
        return true;
      }
    }
  }
  return false;
}

int TruthNodeMaker::FindCommonHitIDs(vector<int>& hitidvec1, vector<int>& hitidvec2)
{
  //This function assumes the input vectors have been sorted
  auto iter = hitidvec1.begin();
  auto jter = hitidvec2.begin();

  int nCommon = 0;
  while(iter != hitidvec1.end() && jter != hitidvec2.end()) {
    if(*iter < *jter) {
      ++iter;
    } else {
      if(!(*jter < *iter)) {
        ++nCommon;
        ++iter;
      }
      ++jter;
    }
  }

  return nCommon;
}
