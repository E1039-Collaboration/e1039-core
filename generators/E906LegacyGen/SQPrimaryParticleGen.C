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
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <Pythia8/Pythia.h>
#include <Pythia8Plugins/HepMC2.h>
#include <phgeom/PHGeomUtility.h>
#include <boost/format.hpp>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <gsl/gsl_randist.h>
#include <Geant4/G4ParticleTable.hh>

#include "SQPrimaryParticleGen.h"
#include "SQMCDimuon.h"
#include "SQPrimaryVertexGen.h"
#include "SQDimuonTruthInfoContainer.h"
#include <iostream>

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

    //distribution-wise constants
    const double pT0DY = 2.8;
    const double pTpowDY = 1./(6. - 1.);
    const double pT0JPsi = 3.0;
    const double pTpowJPsi = 1./(6. - 1.);

    //charmonium generation constants  Ref: Schub et al Phys Rev D 52, 1307 (1995)
    const double sigmajpsi = 0.2398;    //Jpsi xf gaussian width
    const double brjpsi = 0.0594;       //Br(Jpsi -> mumu)
    const double ajpsi = 0.001464*TMath::Exp(-16.66*mjpsi/sqrts);
    const double bjpsi = 2.*sigmajpsi*sigmajpsi;

    const double psipscale = 0.019;     //psip relative to jpsi
  }


SQPrimaryParticleGen::SQPrimaryParticleGen():
  PHG4ParticleGeneratorBase(),
  _Pythia(false),
  _CustomDimuon(false),
  _DrellYanGen(false),
  drellyanMode(false),
  dimuon_info(NULL),
  _JPsiGen(false),
  _PsipGen(false),
  ineve(NULL)
{
 
  _vertexGen = new SQPrimaryVertexGen();
  pdf = LHAPDF::mkPDF("CT10nlo", 0);

}


SQPrimaryParticleGen::~SQPrimaryParticleGen()
{
  delete _vertexGen;
  //delete pdf;

}

int SQPrimaryParticleGen::Init(PHCompositeNode* topNode)
{

  // ppGen.readFile("pythia8_DY.cfg");
  // pnGen.readFile("pythia8_DY.cfg");
  //can't read the configuration file..hard coded for now ..change it back to reading from configuration file
  if(_Pythia){
  ppGen.readString("PDF:pSet = 7 ");//  CTEQ6L
  ppGen.readString("ParticleDecays:limitTau = on"); //Only decays the unstable particles
  ppGen.readString("WeakSingleBoson:ffbar2ffbar(s:gm) = on");// ffbar -> gamma* -> ffbar
  ppGen.readString("Beams:frameType = 2");
  ppGen.readString("Beams:idA = 2212");
  ppGen.readString("Beams:eA = 120.");
  ppGen.readString("Beams:eB = 0.");
  ppGen.readString("Beams:allowVertexSpread = on");


  pnGen.readString("PDF:pSet = 7 ");
  pnGen.readString("ParticleDecays:limitTau = on");
  pnGen.readString("WeakSingleBoson:ffbar2ffbar(s:gm) = on");
  pnGen.readString("Beams:frameType = 2");
  pnGen.readString("Beams:idA = 2212");
  pnGen.readString("Beams:eA = 120.");
  pnGen.readString("Beams:eB = 0.");
  pnGen.readString("Beams:allowVertexSpread = on");



  ppGen.readString("Beams:idB = 2212");
  pnGen.readString("Beams:idB = 2112");
  

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

  ppGen.init();
  pnGen.init();
 
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQPrimaryParticleGen::InitRun(PHCompositeNode* topNode)
{ 
  gRandom->SetSeed(PHRandomSeed());
  ineve = findNode::getClass<PHG4InEvent>(topNode,"PHG4INEVENT");
  if (!ineve) {
    PHNodeIterator iter( topNode );
    PHCompositeNode *dstNode;
    dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
      
    ineve = new PHG4InEvent();
    PHDataNode<PHObject> *newNode = new PHDataNode<PHObject>(ineve, "PHG4INEVENT", "PHObject");
    dstNode->addNode(newNode);

    // Node for storing truth info of Dimuon
    dimuon_info = new SQDimuonTruthInfoContainer();
    PHIODataNode<PHObject> *dimuonNode = new PHIODataNode<PHObject>(dimuon_info, "DimuonInfo", "PHObject");
    dstNode->addNode(dimuonNode);
  }
  
  return 0;
}



int SQPrimaryParticleGen::process_event(PHCompositeNode* topNode)
{

  _vertexGen->InitRun(topNode);
  TGeoManager* geoManager = PHGeomUtility::GetTGeoManager(topNode);
  double x_vtx,y_vtx,z_vtx;
  x_vtx=0.;
  y_vtx=0.;
  z_vtx=0.;
  _vertexGen->traverse(geoManager->GetTopNode(),x_vtx,y_vtx, z_vtx);
  Double_t pARatio = _vertexGen->getPARatio();
  Double_t luminosity =  _vertexGen->getLuminosity();
  TVector3 vtx;
  vtx.SetXYZ(x_vtx,y_vtx,z_vtx);

  if (_DrellYanGen) generateDrellYan(topNode,vtx, pARatio, luminosity);
  if (_Pythia) generatePythia(topNode,vtx, pARatio);
  if (_JPsiGen) generateJPsi(topNode,vtx, pARatio, luminosity);
  if (_PsipGen) generatePsip(topNode,vtx, pARatio, luminosity);
  
  return Fun4AllReturnCodes::EVENT_OK; 
}

//=====================DrellYan=====================================

int SQPrimaryParticleGen::generateDrellYan(PHCompositeNode *topNode,TVector3 vtx, const double pARatio, double luminosity)
{
  
  SQMCDimuon dimuon ;
  drellyanMode = true;
  int vtxindex = -1;
  //sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;

  if(!generateDimuon(mass, xF, dimuon, true)) return Fun4AllReturnCodes::ABORTEVENT; // return
 
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
 
  PHG4Particle *particle_muNeg = new PHG4Particlev2();

  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(dimuon.fNegMomentum.Px());
  particle_muNeg->set_py(dimuon.fNegMomentum.Py());
  particle_muNeg->set_pz(dimuon.fNegMomentum.Pz());
  particle_muNeg->set_e(dimuon.fPosMomentum.E());
  ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();

  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(dimuon.fPosMomentum.Px());
  particle_muplus->set_py(dimuon.fPosMomentum.Py());
  particle_muplus->set_pz(dimuon.fPosMomentum.Pz());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  ineve->AddParticle(vtxindex, particle_muplus);


 
  // Calculate the cross section
  //@cross_section
  // PDF-related cross-section
  //@{
  double zOverA = pARatio;
  double nOverA = 1. - zOverA;

  double dbar1 = pdf->xfxQ(-1, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  double ubar1 = pdf->xfxQ(-2, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  double d1 = pdf->xfxQ(1, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  double u1 = pdf->xfxQ(2, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  double s1 = pdf->xfxQ(3, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  double c1 = pdf->xfxQ(4, dimuon.fx1, dimuon.fMass)/dimuon.fx1;

  double dbar2 = pdf->xfxQ(-1, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double ubar2 = pdf->xfxQ(-2, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double d2 = pdf->xfxQ(1, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double u2 = pdf->xfxQ(2, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double s2 = pdf->xfxQ(3, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double c2 = pdf->xfxQ(4, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
 
  double xsec_pdf = 4./9.*(u1*(zOverA*ubar2 + nOverA*dbar2) + ubar1*(zOverA*u2 + nOverA*d2) + 2*c1*c2) +
                  1./9.*(d1*(zOverA*dbar2 + nOverA*ubar2) + dbar1*(zOverA*d2 + nOverA*u2) + 2*s1*s2);
  //@}

  //KFactor related 
  //@{
  double xsec_kfactor = 1.;
  if(dimuon.fMass < 2.5)
    {
      xsec_kfactor = 1.25;
    }
  else if(dimuon.fMass < 7.5)
    {
      xsec_kfactor = 1.25 + (1.82 - 1.25)*(dimuon.fMass - 2.5)/5.;
    }
  else
    {
      xsec_kfactor = 1.82;
    }
  ///@}
 
  //phase space
  double xsec_phsp = dimuon.fx1*dimuon.fx2/(dimuon.fx1 + dimuon.fx2)/dimuon.fMass/dimuon.fMass/dimuon.fMass;
  
  //generation limitation
  double xsec_limit = (massMax - massMin)*(xfMax - xfMin)*(cosThetaMax*cosThetaMax*cosThetaMax/3. 
							   +cosThetaMax - cosThetaMin*cosThetaMin*cosThetaMin/3. 
							   - cosThetaMin)*4./3.;
  
  
  //Total cross-section
  double xsec = xsec_pdf*xsec_kfactor*xsec_phsp*xsec_limit*luminosity;
  //@cross_section


  //==== Store in dimuon truth container
  dimuon_info->set_Dimuon_xs(xsec);
  dimuon_info->set_Dimuon_m(dimuon.fMass);
  dimuon_info->set_Dimuon_cosThetaCS(dimuon.fCosTh);
  dimuon_info->set_Dimuon_phiCS(dimuon.fPhi);
  dimuon_info->set_Dimuon_pt(dimuon.fpT);
  dimuon_info->set_Dimuon_xF(dimuon.fxF); 

}

//====================generateJPsi===================================================
int SQPrimaryParticleGen::generateJPsi(PHCompositeNode *topNode,TVector3 vtx, const double pARatio, double luminosity)
{
 
  SQMCDimuon dimuon ;
  int vtxindex = -1;
  //sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;

  if(!generateDimuon(DPGEN::mjpsi, xF, dimuon)) return Fun4AllReturnCodes::ABORTEVENT; 
  
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
 
 PHG4Particle *particle_muNeg = new PHG4Particlev2();

  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(dimuon.fNegMomentum.Px());
  particle_muNeg->set_py(dimuon.fNegMomentum.Py());
  particle_muNeg->set_pz(dimuon.fNegMomentum.Pz());
  particle_muNeg->set_e(dimuon.fPosMomentum.E());
  ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(dimuon.fPosMomentum.Px());
  particle_muplus->set_py(dimuon.fPosMomentum.Py());
  particle_muplus->set_pz(dimuon.fPosMomentum.Pz());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  particle_muplus->set_e(dimuon.fPosMomentum.E()); 
  ineve->AddParticle(vtxindex, particle_muplus);

 
  // Calculate the cross section for J/Psi
  //@cross_section{
  //xf distribution
  double xsec_xf = DPGEN::ajpsi*TMath::Exp(-dimuon.fxF*dimuon.fxF/DPGEN::bjpsi)/(DPGEN::sigmajpsi*DPGEN::sqrt2pi);

  //generation limitation
  double xsec_limit = xfMax - xfMin;

  double xsec = DPGEN::brjpsi*xsec_xf*xsec_limit*luminosity;
  //@cross_section}

  //==== Store in dimuon truth container
  dimuon_info->set_Dimuon_xs(xsec);
  dimuon_info->set_Dimuon_m(dimuon.fMass);
  dimuon_info->set_Dimuon_cosThetaCS(dimuon.fCosTh);
  dimuon_info->set_Dimuon_phiCS(dimuon.fPhi);
  dimuon_info->set_Dimuon_pt(dimuon.fpT);
  dimuon_info->set_Dimuon_xF(dimuon.fxF); 
}
//======================Psi-prime====================
int SQPrimaryParticleGen::generatePsip(PHCompositeNode *topNode,TVector3 vtx, const double pARatio, double luminosity)
{
 
  SQMCDimuon dimuon ;
  int vtxindex = -1;
  //sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;

  if(!generateDimuon(DPGEN::mpsip, xF, dimuon)) return Fun4AllReturnCodes::ABORTEVENT; // return; //1;//return 0; 
  
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
 
 PHG4Particle *particle_muNeg = new PHG4Particlev2();

  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(dimuon.fNegMomentum.Px());
  particle_muNeg->set_py(dimuon.fNegMomentum.Py());
  particle_muNeg->set_pz(dimuon.fNegMomentum.Pz());
  particle_muNeg->set_e(dimuon.fPosMomentum.E());
  ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(dimuon.fPosMomentum.Px());
  particle_muplus->set_py(dimuon.fPosMomentum.Py());
  particle_muplus->set_pz(dimuon.fPosMomentum.Pz());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
 
  ineve->AddParticle(vtxindex, particle_muplus);
 
  // Calculate the cross section for J/Psi
  //@cross_section{
  //xf distribution
  double xsec_xf = DPGEN::ajpsi*TMath::Exp(-dimuon.fxF*dimuon.fxF/DPGEN::bjpsi)/(DPGEN::sigmajpsi*DPGEN::sqrt2pi);

  //generation limitation
  double xsec_limit = xfMax - xfMin;

  double xsec = DPGEN::psipscale*DPGEN::brjpsi*xsec_xf*xsec_limit*luminosity;
  //@}


  //==== Store in dimuon truth container
  dimuon_info->set_Dimuon_xs(xsec);
  dimuon_info->set_Dimuon_m(dimuon.fMass);
  dimuon_info->set_Dimuon_cosThetaCS(dimuon.fCosTh);
  dimuon_info->set_Dimuon_phiCS(dimuon.fPhi);
  dimuon_info->set_Dimuon_pt(dimuon.fpT);
  dimuon_info->set_Dimuon_xF(dimuon.fxF); 
}


//==========Pythia Generator====================================================================
int SQPrimaryParticleGen::generatePythia(PHCompositeNode *topNode,TVector3 vtx, const double pARatio)
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


//============Main function to generate dimuon====================
//Reference: G. Moerno et.al. Phys. Rev D43:2815-2836, 1991

bool SQPrimaryParticleGen::generateDimuon(double mass, double xF, SQMCDimuon& dimuon, bool angular)
{
    double pz = xF*(DPGEN::sqrts - mass*mass/DPGEN::sqrts)/2.;
    
    double pTmaxSq = (DPGEN::s*DPGEN::s*(1. - xF*xF) - 2.*DPGEN::s*mass*mass + mass*mass*mass*mass)/DPGEN::s/4.;
  
    if(pTmaxSq < 0.) return false;
    
    double pTmax = sqrt(pTmaxSq);
    double pT = 10.;

    if(pTmax < 0.3)
    {
      pT = pTmax*sqrt(gRandom->Uniform(0,1));
    }
    else if(drellyanMode)
    {
      while(pT > pTmax) pT = DPGEN::pT0DY*TMath::Sqrt(1./TMath::Power(gRandom->Uniform(0,1), DPGEN::pTpowDY) - 1.);
    }
    else
    {
        while(pT > pTmax) pT = DPGEN::pT0JPsi*TMath::Sqrt(1./TMath::Power(gRandom->Uniform(0,1), DPGEN::pTpowJPsi) - 1.);
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
   
    bool firstTry = true;

    // while loop to generate dimuons with 1+cos^2theta distribution
    while(firstTry || angular)
    {
        firstTry = false;

        phaseGen.Generate();
        dimuon.fPosMomentum = *(phaseGen.GetDecay(0));
        dimuon.fNegMomentum = *(phaseGen.GetDecay(1));

        dimuon.calcVariables();
        angular = 2.*gRandom->Uniform(0,1) > 1. + dimuon.fCosTh*dimuon.fCosTh;
    }

  
    if(dimuon.fx1 < x1Min || dimuon.fx1 > x1Max) return false;
    if(dimuon.fx2 < x2Min || dimuon.fx2 > x2Max) return false;
   
    if(dimuon.fCosTh < cosThetaMin || dimuon.fCosTh >cosThetaMax) return false;

    return true;
}
