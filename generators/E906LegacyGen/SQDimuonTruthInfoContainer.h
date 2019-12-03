#ifndef SQDIMUONTRUTHINFOCONTAINER_H
#define SQDIMUONTRUTHINFOCONTAINER_H

#include <phool/PHObject.h>

#include <map>
#include <set>

class SQMCDimuon;

class SQDimuonTruthInfoContainer: public PHObject 
{
 public: 
  //typedef Map::const_iterator ConstIterator;
  
  SQDimuonTruthInfoContainer(){}
  //SQDimuonTruthInfoContainer(const std::string &nodename);
  virtual ~ SQDimuonTruthInfoContainer();
  void Reset();

  double get_Dimuon_xs() const {return Dimuon_xs;}
  double get_Dimuon_m() const {return Dimuon_m;}
  double get_Dimuon_cosThetaCS() const {return Dimuon_cosThetaCS;}
  double get_Dimuon_phiCS() const {return Dimuon_phiCS;}

  void set_Dimuon_xs(const double xs){Dimuon_xs = xs;}
  void set_Dimuon_m(const double m){Dimuon_m = m;}
  void set_Dimuon_cosThetaCS(const double costheta) {Dimuon_cosThetaCS = costheta;}
  void set_Dimuon_phiCS(const double phi) {Dimuon_phiCS = phi;}
  
 private:
  double Dimuon_xs;
  double Dimuon_m;
  double Dimuon_cosThetaCS;
  double Dimuon_phiCS;

  ClassDef(SQDimuonTruthInfoContainer,1)

};

#endif
