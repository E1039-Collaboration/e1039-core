/*====================================================================
Author: Abinash Pun
Nov, 2019 and onwards
Goal: To import the physics generator of E906 experiment from Kun to E1039
=========================================================================*/

#include "E1039PhysicsGen.h"
#include "MCDimuon.h"
#include <fstream>
#include <string>
#include <TRandom3.h>
#include <iostream>


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
#include <gsl/gsl_randist.h>

#include <cassert>
#include <cstdlib>

//====
#include <Geant4/G4ParticleTable.hh>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>

#include "E906VertexGen.h"
#include "BeamlineObject.h"
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
typedef PHIODataNode<PHObject> PHObjectNode_t;




E1039PhysicsGen::E1039PhysicsGen():
  PHG4ParticleGeneratorBase(),
  _PythiaDimuon(false),
  _CustomDimuon(false),
  _DrellYanGen(false),
  drellyanMode(false),
  dimuon_info(NULL),
  ineve(NULL)
{
  //particleGun = new G4ParticleGun(1);
  _vertexGen = new E906VertexGen();
  // const double inch = 2.54;
  // const double mm = 0.1;

  //pdf = LHAPDF::mkPDF("CT10nlo", 0);

}


E1039PhysicsGen::~E1039PhysicsGen()
{
  delete _vertexGen;
  //delete pdf;

}

int E1039PhysicsGen::Init(PHCompositeNode* topNode)
{

  // ppGen.readFile("pythia8_DY.cfg");
  // pnGen.readFile("pythia8_DY.cfg");
  //can't read the configuration file..hard coded for now ..change it back to reading from configuration file
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


  return Fun4AllReturnCodes::EVENT_OK;
}

int E1039PhysicsGen::InitRun(PHCompositeNode* topNode)
{ 

  ineve = findNode::getClass<PHG4InEvent>(topNode,"PHG4INEVENT");
  if (!ineve) {
    PHNodeIterator iter( topNode );
    PHCompositeNode *dstNode;
    dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
      
    ineve = new PHG4InEvent();
    PHDataNode<PHObject> *newNode = new PHDataNode<PHObject>(ineve, "PHG4INEVENT", "PHObject");
    dstNode->addNode(newNode);

    // dimuon_info = new MCDimuon();
    dimuon_info = new SQDimuonTruthInfoContainer();
    PHIODataNode<PHObject> *dimuonNode = new PHIODataNode<PHObject>(dimuon_info, "DimuonInfo", "PHObject");
    dstNode->addNode(dimuonNode);
  }

    //playground to include dimuon node

   // PHNodeIterator iter( topNode );
   //  PHCompositeNode *dstNode;
   //  dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
      
   //  //dimuon_info = new SQDimuonTruthInfoContainer();
   //  dimuon_info = new MCDimuon();
   //  PHDataNode<PHObject> *dimuonNode = new PHDataNode<PHObject>(dimuon_info, "DimuonInfo", "PHObject");
   //  dstNode->addNode(dimuonNode);

  
  return 0;
}



int E1039PhysicsGen::process_event(PHCompositeNode* topNode)
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
  // std::cout<<"pAratio: "<<pARatio<<std::endl;
  //To do: fix the precision of the PARatio when passed via the function
  if (_DrellYanGen) generateDrellYan(topNode,vtx, pARatio, luminosity);
  if (_PythiaDimuon) generatePythia(topNode,vtx, pARatio);
  if (_CustomDimuon) generateCustomDimuon(topNode,vtx, pARatio);
  
  return Fun4AllReturnCodes::EVENT_OK; 
}



int E1039PhysicsGen::generateDrellYan(PHCompositeNode *topNode,TVector3 vtx, const double pARatio, double luminosity)
{
  MCDimuon dimuon ;
  drellyanMode = true;
  int vtxindex = -1;
//sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;
  
  if(!generateDimuon(mass, xF, dimuon, true)) return 1;//return 0;    
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);
  std::cout<<"Inside drellyan generator"<<std::endl;
  PHG4Particle *particle_muplus = new PHG4Particlev2();
  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mup");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(dimuon.fPosMomentum.Px());
  particle_muplus->set_py(dimuon.fPosMomentum.Py());
  particle_muplus->set_pz(dimuon.fPosMomentum.Pz());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  //particle_muplus->set_e(dimuon.fPosMomentum.E());
  particle_muplus->set_e(0.);
  //particle_muplus->set_dimuon_xs(0.0);
  ineve->AddParticle(vtxindex, particle_muplus);

  PHG4Particle *particle_muNeg = new PHG4Particlev2();
  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mum");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(dimuon.fNegMomentum.Px());
  particle_muNeg->set_py(dimuon.fNegMomentum.Py());
  particle_muNeg->set_pz(dimuon.fNegMomentum.Pz());
  particle_muNeg->set_e(dimuon.fPosMomentum.E());
  //particle_muNeg->set_dimuon_xs(0.9);
  ineve->AddParticle(vtxindex, particle_muNeg);

 
  // Calculate the cross section
  //@cross_section
  // PDF-related cross-section
  //@PDF
  double zOverA = pARatio;
  double nOverA = 1. - zOverA;

  // double dbar1 = pdf->xfxQ(-1, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  // double ubar1 = pdf->xfxQ(-2, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  // double d1 = pdf->xfxQ(1, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  // double u1 = pdf->xfxQ(2, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  // double s1 = pdf->xfxQ(3, dimuon.fx1, dimuon.fMass)/dimuon.fx1;
  // double c1 = pdf->xfxQ(4, dimuon.fx1, dimuon.fMass)/dimuon.fx1;

  // double dbar2 = pdf->xfxQ(-1, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  // double ubar2 = pdf->xfxQ(-2, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  // double d2 = pdf->xfxQ(1, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  // double u2 = pdf->xfxQ(2, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  // double s2 = pdf->xfxQ(3, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  // double c2 = pdf->xfxQ(4, dimuon.fx2, dimuon.fMass)/dimuon.fx2;
  double xsec_pdf = 1.0;
  // double xsec_pdf = 4./9.*(u1*(zOverA*ubar2 + nOverA*dbar2) + ubar1*(zOverA*u2 + nOverA*d2) + 2*c1*c2) +
  //                 1./9.*(d1*(zOverA*dbar2 + nOverA*ubar2) + dbar1*(zOverA*d2 + nOverA*u2) + 2*s1*s2);
  //@PDF

  //KFactor related 
  //@Kfactor
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
  ///@Kfactor

  //phase space
  double xsec_phsp = dimuon.fx1*dimuon.fx2/(dimuon.fx1 + dimuon.fx2)/dimuon.fMass/dimuon.fMass/dimuon.fMass;
  
  //generation limitation
  double xsec_limit = (massMax - massMin)*(xfMax - xfMin)*(cosThetaMax*cosThetaMax*cosThetaMax/3. 
							   +cosThetaMax - cosThetaMin*cosThetaMin*cosThetaMin/3. 
							   - cosThetaMin)*4./3.;
  //Total cross-section
  double xsec = xsec_pdf*xsec_kfactor*xsec_phsp*xsec_limit*luminosity;
  //@cross_section


  //====
  MCDimuon* Dimuon_node;
  Dimuon_node->set_Dimuon_xs(0.1);
  dimuon_info->AddDimuon(Dimuon_node);



  return 0;
}


int E1039PhysicsGen::generatePythia(PHCompositeNode *topNode,TVector3 vtx, const double pARatio)
{
  nEventsPhysics++;
  Pythia8::Pythia* p_pythia =gRandom->Uniform(0,1)  < pARatio ? &ppGen : &pnGen ;
  //std::cout<<" Inside physics generator: "<<pARatio<<std::endl;
  int vtxindex = -1;
  int trackid = -1;
 
 
  while(!p_pythia->next()) {}

 
  for(int i = 1; i < p_pythia->event.size(); ++i)
    {
      int pParID = 0;
      Pythia8::Particle par = p_pythia->event[i];
      if(par.status() <= 0 && par.id() == 22) continue; // ignore photons with non-positive status (i.e. unstable)
      
      ++trackid;
      //if(par.id()!=13) continue;
      if(par.mother1() == 0 && par.mother2()==0) continue; // don't store mother particle i.e. colliding protons
      if(par.pz()<5.0) continue;
      if(!(fabs(par.id())==13)) continue;
      pParID++;
      if (pParID>1) continue;
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

int E1039PhysicsGen::generateCustomDimuon(PHCompositeNode *topNode,TVector3 vtx, const double pARatio)
{
  //std::cout<<"pAratio inside funtion: "<<pARatio<<std::endl;
  MCDimuon dimuon;
  //drellyanMode = true;
  int vtxindex = -1;
//sets invaraint mass and xF  = x1-x2 for virtual photon
  double mass = gRandom->Uniform(0,1)*(massMax - massMin) + massMin;
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;
  
  std::cout <<" generateDimuon(): "<<generateDimuon(mass, xF, dimuon, true)<<std::endl;
  if(!generateDimuon(mass, xF, dimuon, true)) return 0;    //TODO: maybe later need to add an option or flag in lut to specify angular distribution
  vtxindex = ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);

  PHG4Particle *particle_muNeg = new PHG4Particlev2();
  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(dimuon.fNegMomentum.Px());
  particle_muNeg->set_py(dimuon.fNegMomentum.Py());
  particle_muNeg->set_pz(dimuon.fNegMomentum.Pz());
  particle_muNeg->set_e(dimuon.fNegMomentum.E());
  //std::cout<<" Energy of mu-: "<<dimuon.fNegMomentum.E()<<std::endl;
  // particle_muNeg->set_px(0.);
  // particle_muNeg->set_py(0.);
  // particle_muNeg->set_pz(0.);
  // particle_muNeg->set_e(0.);
  ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();
  particle_muplus->set_track_id(11);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(dimuon.fPosMomentum.Px());
  particle_muplus->set_py(dimuon.fPosMomentum.Py());
  particle_muplus->set_pz(dimuon.fPosMomentum.Pz());
  particle_muplus->set_e(dimuon.fPosMomentum.E());
  ineve->AddParticle(vtxindex, particle_muplus);


   
  //calculate the cross section
  // double xsec = lut->Interpolate(mass, xF)*p_vertexGen->getLuminosity();
return 0;
}

bool E1039PhysicsGen::generateDimuon(double mass, double xF, MCDimuon& dimuon, bool angular)
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
    double masses[2] = {0.105658,0.105658};//{mup->GetPDGMass()/CLHEP::GeV ,mum->GetPDGMass()/CLHEP::GeV};
    phaseGen.SetDecay(p_dimuon, 2, masses);
   
    bool firstTry = true;
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
