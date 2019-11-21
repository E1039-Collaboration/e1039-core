#include "SQDimuonTruthInfoContainer.h"
#include "MCDimuon.h"

ClassImp(SQDimuonTruthInfoContainer)


SQDimuonTruthInfoContainer::~SQDimuonTruthInfoContainer(){
  Reset();
}

 int SQDimuonTruthInfoContainer::AddDimuon(MCDimuon* dimuon_info)
{
  return 0;
}

void SQDimuonTruthInfoContainer::Reset(){
}
