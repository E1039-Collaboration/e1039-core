#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;

int main(int argc, char* argv[])
{
  ifstream fin(argv[1]);
  string line;

  TFile saveFile(argv[2], "recreate");
  TTree* saveTree = new TTree("fieldmap", "fieldmap");

  float x, y, z, bx, by, bz;

  saveTree->Branch("x", &x, "x/F");
  saveTree->Branch("y", &y, "y/F");
  saveTree->Branch("z", &z, "z/F");
  saveTree->Branch("Bx", &bx, "Bx/F");
  saveTree->Branch("By", &by, "By/F");
  saveTree->Branch("Bz", &bz, "Bz/F");
   
  getline(fin, line); //dummy read for the header
  while(getline(fin, line))
  {
    stringstream ss(line);
    ss >> x >> y >> z >> bx >> by >> bz;

    saveTree->Fill();
  }

  saveFile.cd();
  saveTree->Write();
  saveFile.Close();

  return 0;
}
