#include <iomanip>
#include <phhepmc/PHHepMCGenEventMap.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4VtxPoint.h>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQTrack_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQTrackVector_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "TruthNodeMaker.h"
using namespace std;

TruthNodeMaker::TruthNodeMaker()
  : SubsysReco("TruthNodeMaker")
  , m_evt(0)
  , m_vec_trk(0)
  , m_vec_dim(0)
{
  ;
}

TruthNodeMaker::~TruthNodeMaker()
{
  if (! m_evt    ) delete m_evt;
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
  if (genevtmap->size() != 1) {
    cout << "TruthNodeMaker::process_event(): size != 1 unexpectedly." << endl;
  }
  for (PHHepMCGenEventMap::Iter iter = genevtmap->begin(); iter != genevtmap->end(); ++iter) {
    PHHepMCGenEvent *genevt = iter->second;
    if (! genevt) {
      cout << "No PHHepMCGenEvent object." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
    HepMC::GenEvent *evt = genevt->getEvent();
    if (! evt) {
      cout << "No HepMC::GenEvent object." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
    m_evt->set_process_id(evt->signal_process_id());

    //HepMC::GenVertex* vtx = evt->signal_process_vertex(); // Return 0 as of 2019-11-19.
    HepMC::GenEvent::particle_const_iterator it = evt->particles_begin();
    it++; // Skip the 1st beam particle.
    for (int iii = 0; iii < 4; iii++) {
      it++;
      const HepMC::GenParticle* par = *it;
      const HepMC::FourVector * mom = &par->momentum();
      TLorentzVector lvec;
      lvec.SetPxPyPzE(mom->px(), mom->py(), mom->pz(), mom->e());
      m_evt->set_particle_id(iii, par->pdg_id());
      m_evt->set_particle_momentum(iii, lvec);
    }
  }

  /// Extract the true-track info
  m_vec_trk->clear();
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

    TVector3 pos;
    TLorentzVector mom;
    if (FindHitAtStation(trk_id, g4hc_d1x, &pos, &mom)) {
      trk.set_pos_st1(pos);
      trk.set_mom_st1(mom);
    }

    if (FindHitAtStation(trk_id, g4hc_d3px, &pos, &mom) ||
        FindHitAtStation(trk_id, g4hc_d3mx, &pos, &mom)   ) {
      trk.set_pos_st3(pos);
      trk.set_mom_st3(mom);
    }

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
      dim.set_mom_neg     (trk1->get_mom_vtx());
      dim.set_track_id_pos(trk1->get_track_id());
      dim.set_track_id_neg(trk2->get_track_id());
      m_vec_dim->push_back(&dim);
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
  genevtmap = findNode::getClass<PHHepMCGenEventMap    >(topNode, "PHHepMCGenEventMap");
  g4true    = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  g4hc_d1x  = findNode::getClass<PHG4HitContainer      >(topNode, "G4HIT_D1X");
  g4hc_d3px = findNode::getClass<PHG4HitContainer      >(topNode, "G4HIT_D3pXp");
  g4hc_d3mx = findNode::getClass<PHG4HitContainer      >(topNode, "G4HIT_D3mXp");
  if (! g4hc_d1x) g4hc_d1x = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D0X");

  if (!genevtmap || !g4true || !g4hc_d1x || !g4hc_d3px || !g4hc_d3mx) {
    cout << "Failed at getting nodes: " << genevtmap << " " << g4true << " " << g4hc_d1x << " " << g4hc_d3px << " " << g4hc_d3mx << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
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

  m_evt     = new SQMCEvent_v1();
  m_vec_trk = new SQTrackVector_v1();
  m_vec_dim = new SQDimuonVector_v1();

  node_dst->addNode(new PHIODataNode<PHObject>(m_evt    , "SQMCEvent"          , "PHObject"));
  node_dst->addNode(new PHIODataNode<PHObject>(m_vec_trk, "SQTruthTrackVector" , "PHObject"));
  node_dst->addNode(new PHIODataNode<PHObject>(m_vec_dim, "SQTruthDimuonVector", "PHObject"));

  return Fun4AllReturnCodes::EVENT_OK;
}

bool TruthNodeMaker::FindHitAtStation(const int trk_id, const PHG4HitContainer* g4hc, TVector3* pos, TLorentzVector* mom)
{
  const double M_MU = 0.1056583745; // GeV
  PHG4HitContainer::ConstRange range = g4hc->getHits();
  for (PHG4HitContainer::ConstIterator it = range.first; it != range.second; it++) {
    PHG4Hit* hit = it->second;
    if (hit->get_trkid() == trk_id) {
      pos->SetXYZ (hit->get_x(0)     , hit->get_y(0)     , hit->get_z(0)           );
      mom->SetXYZM(hit->get_px(0),     hit->get_py(0),     hit->get_pz(0), M_MU);
      return true;
    }
  }
  return false;
}
