#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <set>
#include <utility>
#include <map>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>
#include <TMatrixD.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TMath.h>
#include <TRandom3.h>
#include <TChain.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "JobOptsSvc.h"

using namespace std;
typedef pair<int, int> eventPair;

int main(int argc, char *argv[])
{
    //Initialize job options
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    jobOptsSvc->init(argv[1]);
    jobOptsSvc->m_enableOnlineAlignment = false;

    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    TFile* mcFile1 = new TFile(argv[2], "READ");
    TTree* mcTree1 = (TTree*)mcFile1->Get("save");

    SRawMCEvent* mcEvent1 = new SRawMCEvent();
    mcTree1->SetBranchAddress("rawEvent", &mcEvent1);

    TFile* mcFile2 = new TFile(argv[3], "READ");
    TTree* mcTree2 = (TTree*)mcFile2->Get("save");

    SRawMCEvent* mcEvent2 = new SRawMCEvent();
    mcTree2->SetBranchAddress("rawEvent", &mcEvent2);

    TFile* saveFile = new TFile(argv[4], "recreate");
    TTree* saveTree = mcTree1->CloneTree(0);

    int nMCEvents1 = mcTree1->GetEntries();
    int nMCEvents2 = mcTree2->GetEntries();
    int nMergedEvents = nMCEvents1 <= nMCEvents2 ? nMCEvents1 : nMCEvents2;
    for(int i = 0; i < nMergedEvents; ++i)
    {
        mcTree1->GetEntry(i);
        mcTree2->GetEntry(i);

        if(i % 1000 == 0) cout << i << endl;

        mcEvent1->mergeEvent(*mcEvent2);
        saveTree->Fill();

        mcEvent1->clear();
        mcEvent2->clear();
    }

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    return EXIT_SUCCESS;
}
