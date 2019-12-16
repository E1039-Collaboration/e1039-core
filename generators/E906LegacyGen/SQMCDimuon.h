// Authors: Abinash Pun, Kun Liu

#ifndef SQMCDimuon_H
#define SQMCDimuon_H
#include <iostream>
#include <string>
#include <TROOT.h>
#include <TObject.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TString.h>
#include <phool/PHObject.h>

class SQMCDimuon: public PHObject
{
public:
  SQMCDimuon();
  virtual ~SQMCDimuon();
  
  Int_t fDimuonID;
  Int_t fPosTrackID;
  Int_t fNegTrackID;

  TVector3 fVertex;
  TLorentzVector fPosMomentum;
  TLorentzVector fNegMomentum;
  TString fOriginVol;

  bool fAccepted;
  void Reset();
  double get_Dimuon_xs() const {return Dimuon_xs;}

  void set_Dimuon_xs(const double xs){Dimuon_xs = xs;}


 public:
  //! calculate derived variables
  void calcVariables();
  Double_t fMass, fpT, fxF, fx1, fx2, fCosTh, fPhi;
  

 public:
  friend std::ostream& operator << (std::ostream& os, const SQMCDimuon& dimuon);
 private:
  double Dimuon_xs;
  ClassDef(SQMCDimuon, 1)
};
#endif // __MCDimuon_H__
