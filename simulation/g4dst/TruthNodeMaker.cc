#include <iomanip>
#include <phhepmc/PHHepMCGenEventMap.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
//#include <g4main/PHG4HitDefs.h>
#include <g4main/PHG4VtxPoint.h>
//#include <HepMC/GenRanges.h>
//#include <interface_main/SQRun.h>
//#include <interface_main/SQEvent.h>
//#include <interface_main/SQHitVector.h>
//#include <ktracker/SRecEvent.h>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQTrack_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQTrackVector_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
//#include <geom_svc/GeomSvc.h>
//#include <UtilAna/UtilSQHit.h>
#include "TruthNodeMaker.h"
using namespace std;

TruthNodeMaker::TruthNodeMaker()
  : m_evt(0)
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
  //const double M_MU = 0.1056583745; // GeV

  typedef std::map<const HepMC::GenParticle*, int> ParPtr2Id_t;
  ParPtr2Id_t map_par_ptr;

  m_vec_trk->clear();
  m_vec_dim->clear();
  int id_trk = 0; // to be incremented
  int id_dim = 0; // to be incremented

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

    /// Extract the hard process.
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

    /// Extract muons.
    while (++it != evt->particles_end()) {
      const HepMC::GenParticle* par = *it;
      int pid = par->pdg_id();
      if (abs(pid) != 13) continue;
      HepMC::FourVector pos = par->production_vertex()->position();
      HepMC::FourVector mom = par->momentum();

      SQTrack_v1 trk;
      trk.set_track_id(id_trk++);
      trk.set_charge(pid < 0 ? +1: -1); // -13 = mu+
      trk.set_pos_vtx(TVector3      (pos.x(), pos.y(), pos.z()         ));
      trk.set_mom_vtx(TLorentzVector(mom.x(), mom.y(), mom.z(), mom.t()));
      //SetTrackParamUsingHits(par, &trk);
      m_vec_trk->push_back(&trk);

      map_par_ptr[par] = trk.get_track_id();
    }

    /// Extract dimuon vertecies.
    for (HepMC::GenEvent::vertex_const_iterator it = evt->vertices_begin(); it != evt->vertices_end(); it++) {
      HepMC::GenVertex* vtx = *it;
      if (   vtx->particles_in_size () != 1
          || vtx->particles_out_size() != 2) continue;
      HepMC::GenParticle* par_mup = 0;
      HepMC::GenParticle* par_mum = 0;
      for (HepMC::GenVertex::particles_out_const_iterator it_par = vtx->particles_out_const_begin(); it_par != vtx->particles_out_const_end(); it_par++) {
        HepMC::GenParticle* par = *it_par;
        int pdg_id = par->pdg_id();
        if      (pdg_id == -13) par_mup = par;
        else if (pdg_id == +13) par_mum = par;
      }
      if (! par_mup || ! par_mum) continue;
      if (   map_par_ptr.find(par_mup) == map_par_ptr.end() 
          || map_par_ptr.find(par_mum) == map_par_ptr.end()) {
        cout << "ERROR:  GenVertex points to an invalid GenParticle!?" << endl;
        return Fun4AllReturnCodes::ABORTEVENT;
      }
      int idx_mup = map_par_ptr[par_mup];
      int idx_mum = map_par_ptr[par_mum];
      HepMC::GenParticle* par_dim = *(vtx->particles_in_const_begin());
      HepMC::FourVector pos = par_dim->production_vertex()->position();
      HepMC::FourVector mom = par_dim->momentum();

      SQDimuon_v1 dim;
      dim.set_dimuon_id(id_dim++);
      dim.set_pdg_id(par_dim->pdg_id());
      dim.set_pos(TVector3      (pos.x(), pos.y(), pos.z()         ));
      dim.set_mom(TLorentzVector(mom.x(), mom.y(), mom.z(), mom.t()));
      dim.set_mom_pos(m_vec_trk->at(idx_mup)->get_mom_vtx());
      dim.set_mom_neg(m_vec_trk->at(idx_mum)->get_mom_vtx());
      dim.set_track_id_pos(idx_mup);
      dim.set_track_id_neg(idx_mum);
      m_vec_dim->push_back(&dim);
    }
  }

  ///
  /// Construct the true track and dimuon info
  ///
//  m_vec_trk->clear();
//  m_vec_dim->clear();
//  vector<int> vec_vtx_id;
//  for (auto it = g4true->GetPrimaryParticleRange().first; it != g4true->GetPrimaryParticleRange().second; ++it) {
//    PHG4Particle* par = it->second;
//    int pid = par->get_pid();
//    if (abs(pid) != 13) continue; // not muon
//    SQTrack trk;
//    int vtx_id = par->get_vtx_id();
//    PHG4VtxPoint* vtx = g4true->GetVtx(vtx_id);
//    trk.charge = pid < 0 ? +1: -1; // -13 = mu+
//    trk.pos_vtx.SetXYZ(vtx->get_x(), vtx->get_y(), vtx->get_z());
//    trk.mom_vtx.SetXYZM(par->get_px(), par->get_py(), par->get_pz(), M_MU);
//    m_vec_trk->push_back(trk);
//    vec_vtx_id.push_back(vtx_id);
//  }
//
//  unsigned int n_trk = m_vec_trk->size();
//  for (unsigned int i1 = 0; i1 < n_trk; i1++) {
//    SQTrack* trk1 = &m_vec_trk->at(i1);
//    if (trk1->charge <= 0) continue;
//    for (unsigned int i2 = 0; i2 < n_trk; i2++) {
//      SQTrack* trk2 = &m_vec_trk->at(i2);
//      if (trk2->charge >= 0) continue;
//      if (vec_vtx_id[i1] != vec_vtx_id[i2]) continue;
//      SQDimuon dim;
//      dim.pos = trk1->pos_vtx;
//      dim.mom = trk1->mom_vtx + trk2->mom_vtx;
//      dim.mom_pos = trk1->mom_vtx;
//      dim.mom_neg = trk2->mom_vtx;
//      dim.track_id_pos = i1;
//      dim.track_id_neg = i2;
//      m_vec_dim->push_back(dim);
//    }
//  }

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
  //g4hc_d3x  = findNode::getClass<PHG4HitContainer      >(topNode, "G4HIT_D3X");
  if (! g4hc_d1x) g4hc_d1x = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_D0X");

  //if (!genevtmap || !g4true || !g4hc_d1x || !g4hc_d3x) {
  if (!genevtmap || !g4true || !g4hc_d1x) {
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

  node_dst->addNode(new PHIODataNode<PHObject>(m_evt    , "SQMCEvent"         , "PHObject"));
  node_dst->addNode(new PHIODataNode<PHObject>(m_vec_trk, "SQTrueTrackVector" , "PHObject"));
  node_dst->addNode(new PHIODataNode<PHObject>(m_vec_dim, "SQTrueDimuonVector", "PHObject"));

  return Fun4AllReturnCodes::EVENT_OK;
}

//void TruthNodeMaker::SetTrackParamUsingHits(const HepMC::GenParticle* par, SQTrack* trk)
//{
//  PHG4HitContainer::ConstRange range = g4hc_d1x->getHits();
//  for (PHG4HitContainer::ConstIterator it = range.first; it != range.second; it++) {
//    PHG4Hit* hit = it->second;
//    cout << "d1x " << hit->get_detid() << " " << hit->get_trkid() << endl;
//  }
//    
//  //g4hc_d3x;
//}
