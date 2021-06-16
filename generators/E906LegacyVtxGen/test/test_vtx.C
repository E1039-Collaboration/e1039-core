#include <iostream>
#include <TTree.h>
#include <TFile.h>
#include <phool/recoConsts.h>
#include "SQPrimaryVertexGen.h"

int main(int argc, char *argv[])
{
  recoConsts* rc = recoConsts::instance();
  int choice = atoi(argv[1]);
  if(choice == 1)
  {
    rc->set_BoolFlag("TARGETONLY", true);
  }
  else if(choice == 2)
  {
    rc->set_BoolFlag("DUMPONLY", true);
  }

  SQPrimaryVertexGen* vtx = new SQPrimaryVertexGen();
  vtx->InitRun("geom.root");

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