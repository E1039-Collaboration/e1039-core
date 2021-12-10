#include "SQCosmicGen.h"

#include "PHG4Particlev2.h"
#include "PHG4InEvent.h"
#include "PHG4VtxPoint.h"
#include "PHG4TruthInfoContainer.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>

#include <Geant4/G4ParticleTable.hh>
#include <Geant4/G4ParticleDefinition.hh>

#include <TMath.h>

#include <cstdlib>
#include <cmath>
#include <cassert>

SQCosmicGen::SQCosmicGen(const std::string& name): 
  PHG4ParticleGeneratorBase(name),
  _ineve(nullptr),
  _mom_min(1.),
  _mom_max(100.),
  _theta_min(0.),
  _theta_max(0.5*TMath::Pi()),
  _prob_mup(1.3/(1. + 1.3)),
  _prob_mum(1./(1. + 1.3)),
  _p0(55.6),
  _p1(1.04),
  _p2(64.0),
  _altitude(1000.),
  _size_x(800.),
  _size_z(4000.),
  _prob_max(1.),
  _rndm(PHRandomSeed())
{
  _req_st[0] = false;
  _req_st[1] = false;
  _req_st[2] = true;
  _req_st[3] = true;
  _req_st[4] = false;

  _z_st[0] = 620.;
  _z_st[1] = 690.;
  _z_st[2] = 1320.;
  _z_st[3] = 1900.;
  _z_st[4] = 2250.;

  _size_x_st[0] = 102./2.;
  _size_x_st[1] = 152./2.;
  _size_x_st[2] = 233./2.;
  _size_x_st[3] = 320./2.;
  _size_x_st[4] = 368./2.;

  _size_y_st[0] = 122./2.;
  _size_y_st[1] = 137./2.;
  _size_y_st[2] = 264./2.;
  _size_y_st[3] = 166.;
  _size_y_st[4] = 368./2.;
}

int SQCosmicGen::InitRun(PHCompositeNode* topNode) 
{
  _ineve = findNode::getClass<PHG4InEvent>(topNode, "PHG4INEVENT");
  if(!_ineve) 
  {
    PHNodeIterator iter(topNode);
    PHCompositeNode* dstNode;
    dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
      
    _ineve = new PHG4InEvent();
    PHIODataNode<PHObject> *newNode = new PHIODataNode<PHObject>(_ineve, "PHG4INEVENT", "PHObject");
    dstNode->addNode(newNode);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void SQCosmicGen::set_mom_range(const double lo, const double hi)
{
  _mom_min = lo;
  _mom_max = hi;

  if(_mom_min < 1.)
  {
    if(Verbosity() > 2) std::cout << Name() << ": mom_min should be larger than 1 GeV" << std::endl;
    _mom_min = 1.;
  }

  if(_theta_max > 1000.)
  {
    if(Verbosity() > 2) std::cout << Name() << ": mom_max should be smaller than 1 TeV" << std::endl;
    _mom_max = 1000.;
  }

  _prob_max = cosmicProb(_mom_min, 0.);
}

void SQCosmicGen::set_theta_range(const double lo, const double hi)
{
  _theta_min = lo;
  _theta_max = hi;

  if(_theta_min < -0.5*TMath::Pi())
  {
    if(Verbosity() > 2) std::cout << Name() << ": theta_min should be larger than -pi/2." << std::endl;
    _theta_min = -0.5*TMath::Pi();
  }

  if(_theta_max > 0.5*TMath::Pi())
  {
    if(Verbosity() > 2) std::cout << Name() << ": theta_max should be smaller than pi/2." << std::endl;
    _theta_max = 0.5*TMath::Pi();
  }
}

void SQCosmicGen::set_charge_ratio(const double p, const double n)
{
  _prob_mup = p/(p + n);
  _prob_mum = n/(p + n);
}

int SQCosmicGen::process_event(PHCompositeNode* topNode) 
{
  //Generate the charge randomly according to the charge ratio of 1.25
  int pdgcode = _rndm.Rndm() < _prob_mup ? -13 : 13;
  std::string pdgname = get_pdgname(pdgcode);

  //Generate 3-momentum based on the probability function
  int nTries = 0;
  bool cosmic   = false;
  bool accepted = false;
  double x_vtx, z_vtx, y_vtx = _altitude;
  double p, px, py, pz;
  while(!cosmic || !accepted)
  {
    ++nTries;

    p = uniformRand(_mom_min, _mom_max);
    double theta = uniformRand(_theta_min, _theta_max);
    double phi = _rndm.Rndm()*TMath::Pi();
    py = -p*TMath::Cos(theta);
    px =  p*TMath::Sin(theta)*TMath::Cos(phi);
    pz =  p*TMath::Sin(theta)*TMath::Sin(phi);

    cosmic = _rndm.Rndm() < (cosmicProb(p, theta)/_prob_max);
    if(!cosmic) continue;

    accepted = generateVtx(px/pz, py/pz, x_vtx, y_vtx, z_vtx);
  }

  //Generate a muon and set common features
  int vtxID = _ineve->AddVtx(x_vtx, y_vtx, z_vtx, 0.);
  PHG4Particle* particle = new PHG4Particlev2();
  particle->set_track_id(0);
  particle->set_vtx_id(vtxID);
  particle->set_parent_id(0);
  particle->set_pid(pdgcode);
  particle->set_name(pdgname);

  double m  = 0.105658;
  double e  = TMath::Sqrt(p*p + m*m);
  particle->set_px(px);
  particle->set_py(py);
  particle->set_pz(pz);
  particle->set_e(e);
  _ineve->AddParticle(vtxID, particle);  //ownership of particle is transferred to _ineve and released in its Reset action

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT)
  {
    _ineve->identify();
    std::cout << nTries << "  -------------------------------------------------------------------" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void SQCosmicGen::getZVtxLimits(int stationID, double ty, double y, double& min, double& max)
{
  double v1 = (y + ty*_z_st[stationID] + _size_y_st[stationID])/ty;
  double v2 = (y + ty*_z_st[stationID] - _size_y_st[stationID])/ty;
  if(v1 < v2)
  {
    min = std::max(v1, min);
    max = std::min(v2, max);
  }
  else
  {
    min = std::max(v2, min);
    max = std::min(v1, max);
  }
}

void SQCosmicGen::getXVtxLimits(int stationID, double tx, double z, double& min, double& max)
{
  double v1 = -_size_x_st[stationID] - tx*(_z_st[stationID] - z);
  double v2 =  _size_x_st[stationID] - tx*(_z_st[stationID] - z);
  if(v1 < v2)
  {
    min = std::max(v1, min);
    max = std::min(v2, max);
  }
  else
  {
    min = std::max(v2, min);
    max = std::min(v1, max);
  }
}

bool SQCosmicGen::acceptedInSt(int stationID, double x, double y, double z, double tx, double ty)
{
  return fabs(x + tx*(_z_st[stationID] - z)) < _size_x_st[stationID] &&
         fabs(y + ty*(_z_st[stationID] - z)) < _size_y_st[stationID];
}

bool SQCosmicGen::generateVtx(double tx, double ty, double& x, double& y, double& z)
{
  double z_min = -999999.;
  double z_max =  999999.;
  for(int i = 0; i < 5; ++i)
  {
    if(!_req_st[i]) continue;
    getZVtxLimits(i, ty, y, z_min, z_max);
  }
  if(z_max < z_min) return false;
  z = uniformRand(z_min, z_max);

  double x_min = -999999.;
  double x_max =  999999.;
  for(int i = 0; i < 5; ++i)
  {
    if(!_req_st[i]) continue;
    getXVtxLimits(i, tx, z, x_min, x_max);
  }
  if(x_max < x_min) return false;
  x = uniformRand(x_min, x_max);

  for(int i = 0; i < 5; ++i)
  {
    if(!_req_st[i]) continue;
    if(!acceptedInSt(i, x, y, z, tx, ty)) return false;
  }
  return true;
}

double SQCosmicGen::cosmicProb(double p, double theta)
{
  return TMath::Power(_p0, TMath::Cos(_p1*theta))/p/p/_p2;
}

double SQCosmicGen::uniformRand(const double lo, const double hi)
{
  return lo + (hi - lo)*_rndm.Rndm();
}
