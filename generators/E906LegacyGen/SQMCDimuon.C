/*====================================================================
Author: Abinash Pun, Kun Liu
Nov, 2019
Goal: Calcualation of dimuon variables as a part of importing the physics generator of E906 experiment (DPSimPrimaryGeneratorAction)
from Kun to E1039 experiment in Fun4All framework
=========================================================================*/
#include "SQMCDimuon.h"
#include <TMath.h>
ClassImp(SQMCDimuon)

SQMCDimuon::SQMCDimuon() {}


SQMCDimuon::~SQMCDimuon(){
  Reset();
}

void SQMCDimuon::Reset(){
  Dimuon_xs = 1.0;
   }
void SQMCDimuon::calcVariables()
{
    Double_t mp = 0.938;
    Double_t ebeam = 120.;

    TLorentzVector p_beam(0., 0., sqrt(ebeam*ebeam - mp*mp), ebeam);
    TLorentzVector p_target(0., 0., 0., mp);

    TLorentzVector p_cms = p_beam + p_target;
    TLorentzVector p_sum = fPosMomentum + fNegMomentum;

    fMass = p_sum.M();
    fpT = p_sum.Perp();

    fx1 = (p_target*p_sum)/(p_target*p_cms);
    fx2 = (p_beam*p_sum)/(p_beam*p_cms);

    Double_t s = p_cms.M2();
    Double_t sqrts = p_cms.M();
    TVector3 bv_cms = p_cms.BoostVector();
    p_sum.Boost(-bv_cms);

    fxF = 2.*p_sum.Pz()/sqrts/(1. - fMass*fMass/s);
    fCosTh = 2.*(fNegMomentum.E()*fPosMomentum.Pz() - fPosMomentum.E()*fNegMomentum.Pz())/fMass/TMath::Sqrt(fMass*fMass + fpT*fpT);
    fPhi = TMath::ATan(2.*TMath::Sqrt(fMass*fMass + fpT*fpT)/fMass*(fNegMomentum.Px()*fPosMomentum.Py() - fPosMomentum.Px()*fNegMomentum.Py())/(fPosMomentum.Px()*fPosMomentum.Px() - fNegMomentum.Px()*fNegMomentum.Px() + fPosMomentum.Py()*fPosMomentum.Py() - fNegMomentum.Py()*fNegMomentum.Py()));
}
