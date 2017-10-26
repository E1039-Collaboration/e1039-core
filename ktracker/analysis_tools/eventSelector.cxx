#include <iostream>
#include <stdlib.h>
#include <string>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "SRawEvent.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char* argv[])
{
    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    SRawEvent* rawEvent = new SRawEvent();
    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = dataTree->CloneTree(0);

    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);
        if(rawEvent->isTriggeredBy(SRawEvent::NIM3)) saveTree->Fill();

        rawEvent->clear();
    }

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    return EXIT_SUCCESS;
}
