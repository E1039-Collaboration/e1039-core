#include "SQSingleMuonTruthInfo.h"

SQSingleMuonTruthInfo::SQSingleMuonTruthInfo():
  motherPid(0),
  motherMom(0., 0., 0.),
  motherVtx(0., 0., 0.),
  decayLength(-1.),
  muMom(0., 0., 0.),
  muVtx(0., 0., 0.)
{}

SQSingleMuonTruthInfo::~SQSingleMuonTruthInfo()
{
}

void SQSingleMuonTruthInfo::identify(std::ostream& os) const
{
  os << "Mother particle momentum/vertex: " << std::endl;
  os << " PID: " << motherPid << std::endl;
  os << " Mom: "; motherMom.Print();
  os << " Mom: "; motherVtx.Print();

  os << "Decay particle momentum/vertex: " << std::endl;
  os << " Mom: "; muMom.Print();
  os << " Mom: "; muVtx.Print();
  os << " Decay length: " << decayLength << std::endl;
}

void SQSingleMuonTruthInfo::Reset()
{
  motherMom.SetXYZ(0., 0., 0.);
  motherVtx.SetXYZ(0., 0., 0.);
  motherPid = 0;
  muMom.SetXYZ(0., 0., 0.);
  muVtx.SetXYZ(0., 0., 0.);
  decayLength = -1.;
}