#include <iostream>
#include <TTree.h>
#include <TFile.h>
#include "SQPrimaryVertexGen.h"

int main(int argc, char *argv[])
{
  SQPrimaryVertexGen* vtx = new SQPrimaryVertexGen();
  vtx->InitRun("geom.root");

  int choice = atoi(argv[1]);
  if(choice == 1)
  {
    vtx->set_targetOnlyMode();
  }
  else if(choice == 2)
  {
    vtx->set_dumpOnlyMode();
  }

  TFile* saveFile = new TFile(argv[2], "recreate");
  TTree* saveTree = new TTree("save", "save");

  TVector3* v = new TVector3();
  saveTree->Branch("v", &v, 256000, 99);

  for(int i = 0; i < 100000; ++i)
  {
    *v = vtx->generateVertex();
    saveTree->Fill();
  }

  saveFile->cd();
  saveTree->Write();
  saveFile->Close();

  return 0;
}