#ifndef MCDimuon_H
#define MCDimuon_H
#include <iostream>
#include <string>
#include <TROOT.h>
#include <TObject.h>
#include <TVector3.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TString.h>
#include <phool/PHObject.h>

class MCDimuon: public PHObject
{
public:
  MCDimuon();
  virtual ~MCDimuon();
  
  Int_t fDimuonID;
  Int_t fPosTrackID;
  Int_t fNegTrackID;

  TVector3 fVertex;
  TLorentzVector fPosMomentum;
  TLorentzVector fNegMomentum;
  TString fOriginVol;

  bool fAccepted;
  void Reset();
 public:
  //! calculate derived variables
  void calcVariables();
  Double_t fMass, fpT, fxF, fx1, fx2, fCosTh, fPhi;
  
  double get_Dimuon_xs() const {return Dimuon_xs;}

  void set_Dimuon_xs(const double xs){Dimuon_xs = xs;}

 public:
  friend std::ostream& operator << (std::ostream& os, const MCDimuon& dimuon);
 private:
  double Dimuon_xs;
  ClassDef(MCDimuon, 1)
};
#endif // __MCDimuon_H__
