/*====================================================================
Author: Abinash Pun, Kun Liu
Nov, 2019
Goal: Import the physics generator of E906 experiment (DPPrimaryGeneratorAction)
from Kun to E1039 experiment in Fun4All framework
=========================================================================*/
#include <fstream>
#include <string>
#include <TRandom3.h>
#include <iostream>
#include <cassert>
#include <cstdlib>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/recoConsts.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <Pythia8/Pythia.h>
#include <Pythia8Plugins/HepMC2.h>
#include <phgeom/PHGeomUtility.h>
#include <boost/format.hpp>
#include <phhepmc/PHGenIntegralv1.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <gsl/gsl_randist.h>
#include <Geant4/G4ParticleTable.hh>
#include <Geant4/G4PhysicalConstants.hh>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <UtilAna/UtilDimuon.h>

#include "SQPrimaryParticleGen.h"
#include <E906LegacyVtxGen/SQPrimaryVertexGen.h>
#include <cassert>
#include <cstdlib>
using namespace std;

 namespace DPGEN
  {
    // global parameters
    const double pi = TMath::Pi();
    const double twopi = 2.*pi;
    const double sqrt2pi = TMath::Sqrt(twopi);

    // masses
    const double mp = 0.93827;
    const double mmu = 0.10566;
    const double mjpsi = 3.097;
    const double mpsip = 3.686;

    // 4-vectors
    const double ebeam = 120.;
    const TLorentzVector p_beam(0., 0., TMath::Sqrt(ebeam*ebeam - mp*mp), ebeam);
    const TLorentzVector p_target(0., 0., 0., mp);
    const TLorentzVector p_cms = p_beam + p_target;
    const TVector3 bv_cms = p_cms.BoostVector();
    const double s = p_cms.M2();
    const double sqrts = p_cms.M();
  }


SQPrimaryParticleGen::SQPrimaryParticleGen(const std::string& name):
  PHG4ParticleGeneratorBase(name),
  _PythiaGen(false),
  _CustomDimuon(false),
  _DrellYanGen(false),
  _JPsiGen(false),
  _PsipGen(false),
  _vertexGen(0),
  ineve(NULL),
  _configFile("phpythia8.cfg"),
  _evt(0),
  _mcevt(0),
  _vec_dim(0),
  _integral_node(0),
  _dim_gen(new SQDimuon_v1()),
  _pdfset("CT10nlo"),
  _pdf(0),
  _pT0DY    (2.8),
  _pTpowDY  (1./(6. - 1.)),
  _pT0JPsi  (3.0),
  _pTpowJPsi(1./(6. - 1.)),
  _n_gen_acc_evt(0),
  _n_proc_evt(0),
  _weight_sum(0),
  _inte_lumi(0)
{
  ;
}


SQPrimaryParticleGen::~SQPrimaryParticleGen()
{
  if (_dim_gen  ) delete _dim_gen;
  if (_vertexGen) delete _vertexGen;
  //delete _pdf;

}

int SQPrimaryParticleGen::Init(PHCompositeNode* topNode)
{
  _vertexGen = new SQPrimaryVertexGen();
  _pdf = LHAPDF::mkPDF(_pdfset, 0);

  if(_PythiaGen){
  if (!_configFile.empty()) read_config();
  unsigned int seed = PHRandomSeed();
  if (seed > 900000000)
    {
      seed = seed % 900000000;
    }

  if ((seed > 0) && (seed <= 900000000))
    {
      ppGen.readString("Random:setSeed = on");
      ppGen.readString(str(boost::format("Random:seed = %1%") % seed));

      pnGen.readString("Random:setSeed = on");
      pnGen.readString(str(boost::format("Random:seed = %1%") % seed));
      
    }

  ppGen.readString("Beams:idB = 2212");
  pnGen.readString("Beams:idB = 2112");

  ppGen.init();
  pnGen.init();
 
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQPrimaryParticleGen::InitRun(PHCompositeNode* topNode)
{ 
  gRandom->SetSeed(PHRandomSeed());

  PHNodeIterator iter( topNode );
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode) {
    cout << PHWHERE << "DST Node missing.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  ineve = findNode::getClass<PHG4InEvent>(topNode,"PHG4INEVENT");
  if (!ineve) {
    ineve = new PHG4InEvent();
    dstNode->addNode(new PHIODataNode<PHObject>(ineve, "PHG4INEVENT", "PHObject"));
  }
  _evt = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! _evt) {
    _evt = new SQEvent_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_evt, "SQEvent", "PHObject"));
  }
  _mcevt = findNode::getClass<SQMCEvent>(topNode, "SQMCEvent");
  if (! _mcevt) {
    _mcevt = new SQMCEvent_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_mcevt, "SQMCEvent", "PHObject"));
  }
  _vec_dim = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if (! _vec_dim) {
    _vec_dim = new SQDimuonVector_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_vec_dim, "SQTruthDimuonVector", "PHObject"));
  }

  PHCompositeNode *runNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!runNode) {
    cout << PHWHERE << "RUN Node missing.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  _integral_node = findNode::getClass<PHGenIntegral>(runNode, "PHGenIntegral");
  if (!_integral_node) {
    _integral_node = new PHGenIntegralv1("By SQPrimaryParticleGen");
    runNode->addNode(new PHIODataNode<PHObject>(_integral_node, "PHGenIntegral", "PHObject"));
  } else {
    cout << PHWHERE << "PHGenIntegral Node exists.  Unexpected.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  _vertexGen->InitRun(topNode);
  
  return 0;
}

int SQPrimaryParticleGen::End(PHCompositeNode* topNode)
{
  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag   ("E906GEN_EVENT_COUNT", _n_proc_evt);
  rc->set_CharFlag  ("E906GEN_PDFSET"     , _pdfset);
  rc->set_DoubleFlag("E906GEN_xfMin"      , xfMin);
  rc->set_DoubleFlag("E906GEN_xfMax"      , xfMax);
  rc->set_DoubleFlag("E906GEN_massMin"    , massMin);
  rc->set_DoubleFlag("E906GEN_massMax"    , massMax);

  if (_PythiaGen) {
    rc->set_CharFlag("E906GEN_CHANNEL", "Pythia");
    rc->set_CharFlag("E906GEN_PYTHIA8_CONFIG", _configFile);
  } else if (_CustomDimuon) {
    rc->set_CharFlag("E906GEN_CHANNEL", "Custom");
  } else if (_DrellYanGen) {
    rc->set_CharFlag  ("E906GEN_CHANNEL", "DrellYan");
    rc->set_DoubleFlag("E906GEN_pT0DY"  , _pT0DY  );
    rc->set_DoubleFlag("E906GEN_pTpowDY", _pTpowDY);
  } else if (_JPsiGen) {
    rc->set_CharFlag("E906GEN_CHANNEL", "Jpsi");
    rc->set_DoubleFlag("E906GEN_pT0JPsi"  , _pT0JPsi  );
    rc->set_DoubleFlag("E906GEN_pTpowJPsi", _pTpowJPsi);
  } else if (_PsipGen) {
    rc->set_CharFlag("E906GEN_CHANNEL", "Psip");
    rc->set_DoubleFlag("E906GEN_pT0JPsi"  , _pT0JPsi  );
    rc->set_DoubleFlag("E906GEN_pTpowJPsi", _pTpowJPsi);
  } else {
    rc->set_CharFlag("E906GEN_CHANNEL", "Undefined");
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQPrimaryParticleGen::process_event(PHCompositeNode* topNode)
{
  TVector3 vtx        = _vertexGen->generateVertex();
  Double_t pARatio    = _vertexGen->getPARatio();
  Double_t luminosity = _vertexGen->getLuminosity();

  int ret = 1; // Error status by default
  if (_DrellYanGen) ret = generateDrellYan(vtx, pARatio, luminosity);
  if (_PythiaGen  ) ret = generatePythia(vtx, pARatio);
  if (_JPsiGen    ) ret = generateJPsi(vtx, pARatio, luminosity);
  if (_PsipGen    ) ret = generatePsip(vtx, pARatio, luminosity);
  if (ret != 0) return Fun4AllReturnCodes::ABORTEVENT;

  _n_gen_acc_evt++;
  _n_proc_evt++;
  _weight_sum += _mcevt->get_weight();
  _inte_lumi  += luminosity;
  _integral_node->set_N_Generator_Accepted_Event(_n_gen_acc_evt);
  _integral_node->set_N_Processed_Event         (_n_proc_evt);
  _integral_node->set_Sum_Of_Weight             (_weight_sum);
  _integral_node->set_Integrated_Lumi           (_inte_lumi);

  _evt->set_run_id  (0);
  _evt->set_spill_id(0);
  _evt->set_event_id(_n_proc_evt);

  return Fun4AllReturnCodes::EVENT_OK; 
}

//=====================DrellYan=====================================
/// Calculate the Drell-Yan cross section, d^2sigma/dM*dxF in pb/GeV
double SQPrimaryParticleGen::CrossSectionDrellYan(const double mass, const double xF, const double x1, const double x2, const double pARatio)
{
  double zOverA = pARatio;
  double nOverA = 1. - zOverA;

  double dbar1 = _pdf->xfxQ(-1, x1, mass)/x1;
  double ubar1 = _pdf->xfxQ(-2, x1, mass)/x1;
  double d1    = _pdf->xfxQ( 1, x1, mass)/x1;
  double u1    = _pdf->xfxQ( 2, x1, mass)/x1;
  double s1    = _pdf->xfxQ( 3, x1, mass)/x1;
  double c1    = _pdf->xfxQ( 4, x1, mass)/x1;

  double dbar2 = _pdf->xfxQ(-1, x2, mass)/x2;
  double ubar2 = _pdf->xfxQ(-2, x2, mass)/x2;
  double d2    = _pdf->xfxQ( 1, x2, mass)/x2;
  double u2    = _pdf->xfxQ( 2, x2, mass)/x2;
  double s2    = _pdf->xfxQ( 3, x2, mass)/x2;
  double c2    = _pdf->xfxQ( 4, x2, mass)/x2;
 
  double xsec_pdf = 4./9.*(u1*(zOverA*ubar2 + nOverA*dbar2) + ubar1*(zOverA*u2 + nOverA*d2) + 2*c1*c2) +
                    1./9.*(d1*(zOverA*dbar2 + nOverA*ubar2) + dbar1*(zOverA*d2 + nOverA*u2) + 2*s1*s2);

  // KFactor
  double xsec_kfactor;
  if      (mass < 2.5) xsec_kfactor = 1.25;
  else if (mass < 7.5) xsec_kfactor = 1.25 + (1.82 - 1.25)*(mass - 2.5)/5.;
  else                 xsec_kfactor = 1.82;
 
  //phase space
  double mass_MeV = mass * (CLHEP::GeV/CLHEP::MeV);
  double xsec_phsp = x1*x2/(x1 + x2)/pow(mass_MeV, 3);
  
  //generation limitation
  double xsec_limit = (pow(cosThetaMax, 3)/3 + cosThetaMax - pow(cosThetaMin, 3)/3 - cosThetaMin) * 3/8;

  // hbarc_squared is in MeV*mm^2, given by Geant4.
  const double xsec_const = 8.0 / 9.0 * pi * pow(CLHEP::fine_structure_const, 2) * CLHEP::hbarc_squared;
  
  // Cross section in pb/GeV
  return xsec_pdf * xsec_kfactor * xsec_phsp * xsec_limit * xsec_const * (CLHEP::mm2/CLHEP::picobarn) / (CLHEP::MeV/CLHEP::GeV);
}

/// Calculate the Drell-Yan cross section, given only mass, xF and pARatio
/**
 * This function is intended to be used in checking the cross-section values computed by this generator.
 * See macros/CheckParticleGen.C.
 */
double SQPrimaryParticleGen::CrossSectionDrellYan(const double mass, const double xF, const double pARatio)
{
  if (!generateDimuon(mass, xF)) {
    cout << PHWHERE << "Failed at generating a dimuon.  Unexpected." << endl;
    exit(1);
  }
  double dim_mass, dim_pT, dim_x1, dim_x2, dim_xF;
  UtilDimuon::CalcVar(_dim_gen, dim_mass, dim_pT, dim_x1, dim_x2, dim_xF);
  return CrossSectionDrellYan(dim_mass, dim_xF, dim_x1, dim_x2, pARatio);
}

/// Calculate the cross section for J/psi vs xF in pb.  Return "BR*sigma(xF)".
/**
 * The cross section is parameterized as "BR*sigma = A * exp(-(xF/W)^2/2) / (sqrt(2*pi)*W)".
 * The "A" term includes the sqrt(s) dependence.
 * The remaining term includes the xF dependence using the Gaussian shape.
 * The constants were taken from "Schub et al., Phys Rev D 52, 1307 (1995)".
 */
double SQPrimaryParticleGen::CrossSectionJPsi(const double xF)
{
  const double A = 1.464e6*TMath::Exp(-16.66*DPGEN::mjpsi/DPGEN::sqrts); // Taken from Tab. 1 of PRD52_1307
  const double W = 0.2398; // Jpsi xf gaussian width. Taken from where?
  const double BR = 0.0594; // Branching ratio of J/psi -> mumu.  Seen above Fig. 4 of PRD52_1307.
  return BR * A * TMath::Exp(-xF*xF/W/W/2) / (DPGEN::sqrt2pi * W);
}

/// Calculate the cross section for psi' vs xF in pb.
/**
 * It just returns the cross section of J/psi scaled by the psi'-to-J/psi ratio, "psipscale".
 * "psipscale" was taken from line 5 in page 1313 of PRD52_1307, 
 * although the value is slightly different (0.019 vs 0.018).
 */
double SQPrimaryParticleGen::CrossSectionPsip(const double xF)
{
  const double psipscale = 0.019; // psi' relative to J/psi in dimuon channel
  return psipscale * CrossSectionJPsi(xF);
}

int SQPrimaryParticleGen::generateDrellYan(const TVector3& vtx, const double pARatio, double luminosity)
{
  //sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;

  if(!generateDimuon(mass, xF)) return 1;
 
  InsertMuonPair(vtx);

  double dim_mass, dim_pT, dim_x1, dim_x2, dim_xF;
  UtilDimuon::CalcVar(_dim_gen, dim_mass, dim_pT, dim_x1, dim_x2, dim_xF);

  double xsec   = CrossSectionDrellYan(dim_mass, dim_xF, dim_x1, dim_x2, pARatio);
  double weight = xsec * luminosity * (massMax - massMin) * (xfMax - xfMin);
  InsertEventInfo(xsec, weight, vtx);

  return 0;
}

//====================generateJPsi===================================================
int SQPrimaryParticleGen::generateJPsi(const TVector3& vtx, const double pARatio, double luminosity)
{
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;
  if(!generateDimuon(DPGEN::mjpsi, xF)) return 1;
  InsertMuonPair(vtx);
  double xsec   = CrossSectionJPsi(xF);
  double weight = xsec * luminosity * (xfMax - xfMin);
  InsertEventInfo(xsec, weight, vtx);
  return 0;
}

//======================Psi-prime====================
int SQPrimaryParticleGen::generatePsip(const TVector3& vtx, const double pARatio, double luminosity)
{
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;
  if(!generateDimuon(DPGEN::mpsip, xF)) return 1;
  InsertMuonPair(vtx);
  double xsec   = CrossSectionPsip(xF);
  double weight = xsec * luminosity * (xfMax - xfMin);
  InsertEventInfo(xsec, weight, vtx);
  return 0;
}


//==========Pythia Generator====================================================================
int SQPrimaryParticleGen::generatePythia(const TVector3& vtx, const double pARatio)
{
 
  Pythia8::Pythia* p_pythia =gRandom->Uniform(0,1)  < pARatio ? &ppGen : &pnGen ;

  int vtxindex = -1;
  int trackid = -1;
  while(!p_pythia->next()) {}

 
  for(int i = 1; i < p_pythia->event.size(); ++i)
    {
      int pParID = 0;
      Pythia8::Particle par = p_pythia->event[i];
      if(par.status() <= 0 && par.id() == 22) continue; // ignore photons with non-positive status (i.e. unstable)
      
      ++trackid;

      if(par.mother1() == 0 && par.mother2()==0) continue; // don't store mother particle i.e. colliding protons
      if(par.pz()<5.0) continue; // momentum cut
      // if(!(fabs(par.id())==13)) continue;
      // pParID++;
      //if (pParID>2) continue;
      vtxindex = ineve->AddVtx(vtx.X()+(par.xProd()*CLHEP::mm),vtx.Y()+(par.yProd()*CLHEP::mm),vtx.Z()+(par.zProd()*CLHEP::mm),0.);

      PHG4Particle *particle = new PHG4Particlev2();
      particle->set_track_id(trackid);
      particle->set_vtx_id(vtxindex);
      particle->set_name(par.name());
      particle->set_pid(par.id());
      particle->set_px(par.px());
      particle->set_py(par.py());
      particle->set_pz(par.pz());
      particle->set_e(par.e());
      ineve->AddParticle(vtxindex, particle);
      
    }

  return 0;
}

int SQPrimaryParticleGen::read_config(const char *cfg_file)
{
  if (cfg_file) _configFile = cfg_file;

  if (verbosity >= VERBOSITY_SOME)
    cout << "PHPythia8::read_config - Reading " << _configFile << endl;

  ifstream infile(_configFile.c_str());

  if (infile.fail())
  {
    cout << "PHPythia8::read_config - Failed to open file " << _configFile << endl;
    exit(2);
  }

  ppGen.readFile(_configFile.c_str());
  pnGen.readFile(_configFile.c_str());

  return Fun4AllReturnCodes::EVENT_OK;
}

/// Main function to generate dimuon
/**
 * Reference: G. Moerno et.al. Phys. Rev D43:2815-2836, 1991
 * 
 * The pT and theta distributions generated are affected by the value of `_DrellYanGen`.
 * The formula of pTmaxSq is discussed in DocDB 9292.
 */
bool SQPrimaryParticleGen::generateDimuon(double mass, double xF)
{
    double pz = xF*(DPGEN::sqrts - mass*mass/DPGEN::sqrts)/2.;
    
    //double pTmaxSq = (DPGEN::s*DPGEN::s*(1. - xF*xF) - 2.*DPGEN::s*mass*mass + mass*mass*mass*mass)/DPGEN::s/4.;
    double pTmaxSq = (DPGEN::s / 4) * (1 - mass*mass/DPGEN::s)*(1 - mass*mass/DPGEN::s) * (1 - xF*xF);
    if(pTmaxSq < 0.)
    {
      cout << PHWHERE << "pTmaxSq < 0." << endl;
      return false;
    }
    
    double pTmax = sqrt(pTmaxSq);
    double pT = 10.;

    if(pTmax < 0.3)
    {
      pT = pTmax*sqrt(gRandom->Uniform(0,1));
    }
    else if(_DrellYanGen) // PRD43, 2815 (1991)
    {
      while(pT > pTmax) pT = _pT0DY*TMath::Sqrt(1./TMath::Power(gRandom->Uniform(0,1), _pTpowDY) - 1.);
    }
    else
    {
      while(pT > pTmax) pT = _pT0JPsi*TMath::Sqrt(1./TMath::Power(gRandom->Uniform(0,1), _pTpowJPsi) - 1.);
    }

    double phi = gRandom->Uniform(0,1)*DPGEN::twopi;
    double px = pT*TMath::Cos(phi);
    double py = pT*TMath::Sin(phi);
   
    //configure phase space generator
    TLorentzVector p_dimuon;
    p_dimuon.SetXYZM(px, py, pz, mass);
    p_dimuon.Boost(DPGEN::bv_cms);
    double masses[2] = {0.105658,0.105658};// Mass of muons
    phaseGen.SetDecay(p_dimuon, 2, masses);

    const int N_TRY = 1000000;
    for (int i_try = 0; i_try < N_TRY; i_try++) // Loop to generate a proper kinematics.
    {
        phaseGen.Generate();
        _dim_gen->set_mom_pos(*(phaseGen.GetDecay(0)));
        _dim_gen->set_mom_neg(*(phaseGen.GetDecay(1)));
        double dim_mass, dim_pT, dim_x1, dim_x2, dim_xF;
        UtilDimuon::CalcVar(_dim_gen, dim_mass, dim_pT, dim_x1, dim_x2, dim_xF);
        if(dim_x1 < x1Min || dim_x1 > x1Max) continue;
        if(dim_x2 < x2Min || dim_x2 > x2Max) continue;
        double dim_costh, dim_phi;
        UtilDimuon::Lab2CollinsSoper(_dim_gen, dim_costh, dim_phi);
        if(dim_costh < cosThetaMin || dim_costh >cosThetaMax) continue;
        if(_DrellYanGen  &&  gRandom->Uniform(0,2) > 1. + pow(dim_costh, 2)) continue;
        return true; // A proper dimuon is generated.
    }
    cout << PHWHERE << "No proper dimuon was generated.  Is your kinematic range reasonable?" << endl;
    return false;
}

/// Insert PHG4Particles objects of mu+ and mu- into "ineve".
/**
 *  Note that "ineve->AddVtx()" was called twice in generateJPsi() and generatePsip() 
 *  in the past versions since 2020-11-05.  But it seems not proper because a muon pair
 *   should share one vertex.
 */
void SQPrimaryParticleGen::InsertMuonPair(const TVector3& vtx)
{
  int vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);

  PHG4Particle *particle_muNeg = new PHG4Particlev2();
  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(_dim_gen->get_mom_neg().Px());
  particle_muNeg->set_py(_dim_gen->get_mom_neg().Py());
  particle_muNeg->set_pz(_dim_gen->get_mom_neg().Pz());
  particle_muNeg->set_e (_dim_gen->get_mom_neg().E ());
  ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();
  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(_dim_gen->get_mom_pos().Px());
  particle_muplus->set_py(_dim_gen->get_mom_pos().Py());
  particle_muplus->set_pz(_dim_gen->get_mom_pos().Pz());
  particle_muplus->set_e (_dim_gen->get_mom_pos().E ());
  ineve->AddParticle(vtxindex, particle_muplus);
}

/// Insert the event info into SQ interface objects.
/**
 * This function could be merged to InsertMuonPair().
 */
void SQPrimaryParticleGen::InsertEventInfo(const double xsec, const double weight, const TVector3& vtx)
{
  static int dim_id = 0;

  _mcevt->set_cross_section(xsec);
  _mcevt->set_weight       (weight);

  _vec_dim->clear();
  _dim_gen->set_dimuon_id   (++dim_id);
  _dim_gen->set_pos         (vtx);
  _dim_gen->set_mom         (_dim_gen->get_mom_pos() + _dim_gen->get_mom_neg());
  _dim_gen->set_track_id_pos( 2); // Given in InsertMuonPair().
  _dim_gen->set_track_id_neg(12); // Given in InsertMuonPair().
  _vec_dim->push_back(_dim_gen);
}
