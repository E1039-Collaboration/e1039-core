#ifndef SQDIMUONTRUTHINFOCONTAINER_H
#define SQDIMUONTRUTHINFOCONTAINER_H

#include <phool/PHObject.h>

#include <map>
#include <set>

class MCDimuon;

class SQDimuonTruthInfoContainer: public PHObject 
{
 public: 
  //typedef Map::const_iterator ConstIterator;
  
  SQDimuonTruthInfoContainer(){}
  //SQDimuonTruthInfoContainer(const std::string &nodename);
  virtual ~ SQDimuonTruthInfoContainer();
  void Reset();
  int AddDimuon(MCDimuon* dimuon_info);
  // ConstIterator AddDimuon(MCDimuon *newdimuon);
  //double get_Dimuon_xs() const {return Dimuon_xs;}

  //void set_Dimuon_xs(const double xs){Dimuon_xs = xs;}

 private:
  //double Dimuon_xs;

  ClassDef(SQDimuonTruthInfoContainer,1)

};

#endif
