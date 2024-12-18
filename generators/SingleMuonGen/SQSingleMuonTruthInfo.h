#ifndef _SQSingleMuonTruthInfo_H
#define _SQSingleMuonTruthInfo_H

#include <phool/PHObject.h>

#include <iostream>
#include <TLorentzVector.h>

class SQSingleMuonTruthInfo: public PHObject 
{
public:
  SQSingleMuonTruthInfo();
  virtual ~SQSingleMuonTruthInfo();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const { return 1; };
  SQSingleMuonTruthInfo* Clone() const { return (new SQSingleMuonTruthInfo(*this)); }

public:
  int motherPid;
  TVector3 motherMom;
  TVector3 motherVtx;

  double decayLength;
  TVector3 muMom;
  TVector3 muVtx;

private:
  ClassDef(SQSingleMuonTruthInfo, 1)

};

#endif
