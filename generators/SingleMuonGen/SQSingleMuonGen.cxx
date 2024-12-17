#include "SQSingleMuonGen.h"

#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4VtxPointv1.h>
#include <g4main/PHG4TruthInfoContainer.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>

#include <Geant4/G4ParticleTable.hh>
#include <Geant4/G4ParticleDefinition.hh>

#include <TMath.h>
#include <TGenPhaseSpace.h>

#include <cstdlib>
#include <cmath>
#include <cassert>

SQSingleMuonGen::SQSingleMuonGen(const std::string& name): 
  PHG4ParticleGeneratorBase(name),
  _ineve(nullptr),
  _truth(nullptr),
  _target_only(true),
  _enable_geom_cut(true),
  _enable_real_mom_dist(true),
  _ctau(2.6E-8*29979245800.),
  _mom_min(1.),
  _mom_max(100.),
  _pt_max(4.),
  _charge_ratio(0.5),
  _m_muon(0.105658),
  _m_pion(0.13957),
  _rndm(PHRandomSeed())
{
}

int SQSingleMuonGen::InitRun(PHCompositeNode* topNode) 
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

  _truth = new SQSingleMuonTruthInfo();
  PHIODataNode<PHObject>* truthNode = new PHIODataNode<PHObject>(_truth, "TruthInfo", "PHObject");
  dstNode->addNode(truthNode);

  _ineve = findNode::getClass<PHG4InEvent>(topNode, "PHG4INEVENT");
  if(!_ineve) 
  {  
    _ineve = new PHG4InEvent();
    PHIODataNode<PHObject>* eveNode = new PHIODataNode<PHObject>(_ineve, "PHG4INEVENT", "PHObject");
    dstNode->addNode(eveNode);
  }

  //_pt_dist    = new TF1("PtDistr", "1*(x^-2)", 0.00001, _pt_max);
  _pt_dist    = new TF1("PtDistr", "TMath::Landau(x, 0.416742, 0.174227)", 0., _pt_max);
  _pz_dist[0] = new TF1("PzDistr1", "0.161*(x^-2.396)", _mom_min, _mom_max);
  _pz_dist[1] = new TF1("PzDistr2", "0.527*(x^-1.996)", _mom_min, _mom_max);
  _pz_dist[2] = new TF1("PzDistr3", "0.489*(x^-2.200)", _mom_min, _mom_max);
  _pz_dist[3] = new TF1("PzDistr4", "0.302*(x^-1.900)", _mom_min, _mom_max);
  _pz_dist[4] = new TF1("PzDistr5", "0.174*(x^-1.750)", _mom_min, _mom_max);
  _pz_dist[5] = new TF1("PzDistr6", "0.909*(x^-1.850)", _mom_min > 8. ? _mom_min : 8., _mom_max);

  return Fun4AllReturnCodes::EVENT_OK;
}

SQSingleMuonGen::~SQSingleMuonGen()
{
  for(int i = 0; i < 6; ++i) delete _pz_dist[i];
  delete _pt_dist;
}

int SQSingleMuonGen::process_event(PHCompositeNode* topNode) 
{
  int nTries = 0;
  bool accepted = false;
  while(!accepted)
  {
    //Generate mother particle first
    generatePrimaryVtx();
    generateMotherParticle();

    //Decay the mother particle, and get the muon's vtx and mom
    decayMotherParticle();

    accepted = _enable_geom_cut ? geometryCut() : true;

    ++nTries;
  }

  if(Verbosity() > 10) std::cout << "Generated single muon within acceptance after " << nTries << " tries." << std::endl;

  int pdgcode = _truth->motherPid > 0 ? 13 : -13;
  std::string pdgname = get_pdgname(pdgcode);

  //Generate a muon and set common features
  int vtxID = _ineve->AddVtx(_truth->muVtx.X(), _truth->muVtx.Y(), _truth->muVtx.Z(), 0.);
  PHG4Particle* particle = new PHG4Particlev2();
  particle->set_track_id(0);
  particle->set_vtx_id(vtxID);
  particle->set_parent_id(0);
  particle->set_pid(pdgcode);
  particle->set_name(pdgname);

  double e  = TMath::Sqrt(_truth->muMom.Mag2() + _m_muon*_m_muon);
  particle->set_px(_truth->muMom.X());
  particle->set_py(_truth->muMom.Y());
  particle->set_pz(_truth->muMom.Z());
  particle->set_e(e);
  _ineve->AddParticle(vtxID, particle);  //ownership of particle is transferred to _ineve and released in its Reset action

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT)
  {
    _ineve->identify();
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void SQSingleMuonGen::generateMotherParticle()
{
  int pid = _rndm.Rndm() < _charge_ratio ? 211 : -211;
  std::string name = get_pdgname(pid);

  TLorentzVector p;
  if(_enable_real_mom_dist)
  {
    double pt = _pt_dist->GetRandom();
    double pz = 0.;
    if(pt < 0.1)
    {
      pz = _pz_dist[0]->GetRandom();
    }
    else if(pt < 0.2)
    {
      pz = _pz_dist[1]->GetRandom();
    }
    else if(pt < 0.3)
    {
      pz = _pz_dist[2]->GetRandom();
    }
    else if(pt < 0.4)
    {
      pz = _pz_dist[3]->GetRandom();
    }
    else if(pt < 0.5)
    {
      pz = _pz_dist[4]->GetRandom();
    }
    else
    {
      pz = _pz_dist[5]->GetRandom();
    }

    double phi = _rndm.Rndm()*TMath::TwoPi();
    double px = pt*TMath::Cos(phi);
    double py = pt*TMath::Sin(phi);

    p.SetXYZM(px, py, pz, _m_pion);
  }
  else
  {
    double pz = _mom_min + (_mom_max - _mom_min)*_rndm.Rndm();
    double pt = _rndm.Rndm()*_pt_max;
    double phi = _rndm.Rndm()*TMath::TwoPi();
    double px = pt*TMath::Cos(phi);
    double py = pt*TMath::Sin(phi);

    p.SetXYZM(px, py, pz, _m_pion);
  }

  _truth->motherPid = pid;
  _truth->motherMom = p.Vect();
}

void SQSingleMuonGen::decayMotherParticle()
{
  TGenPhaseSpace evtgen;
  double m[2] = {_m_muon, 0.};

  TLorentzVector W;
  W.SetVectM(_truth->motherMom, _m_pion);
  evtgen.SetDecay(W, 2, m);
  evtgen.Generate();

  TLorentzVector* p = evtgen.GetDecay(0);
  _truth->muMom = p->Vect();

  // Get the decay vector
  double ctau1  = _ctau*W.Gamma();
  _truth->decayLength = _rndm.Exp(ctau1);
  _truth->muVtx = _truth->decayLength*_truth->muMom.Unit() + _truth->motherVtx;
  
  _truth->identify();
}

bool SQSingleMuonGen::geometryCut()
{
  if(fabs(_truth->muMom.Px()/_truth->muMom.Pz()) > 0.15) return false;
  if(fabs(_truth->muMom.Py()/_truth->muMom.Pz()) > 0.15) return false;
  if(_truth->muMom.Mag() < 10.) return false;

  if(_truth->decayLength > 300.) return false;
  return true;
}

bool SQSingleMuonGen::generatePrimaryVtx()
{
  double x = 0.;
  double y = 0.;
  double z = -300.;

  _truth->motherVtx.SetXYZ(x, y, z);

  return true;
}
