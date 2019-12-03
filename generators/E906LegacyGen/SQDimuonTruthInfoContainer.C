/*====================================================================
Author: Abinash Pun, Kun Liu
Nov, 2019
Goal: To store the dimuon truth information in node
Part of importing the physics generator of E906 experiment (DPSimPrimaryGeneratorAction)
from Kun to E1039 experiment in Fun4All framework
=========================================================================*/
#include "SQDimuonTruthInfoContainer.h"
#include "SQMCDimuon.h"

ClassImp(SQDimuonTruthInfoContainer)


SQDimuonTruthInfoContainer::~SQDimuonTruthInfoContainer(){
  Reset();
}

void SQDimuonTruthInfoContainer::Reset(){
}
