#include <iostream>
#include <stdlib.h>
#include <string>
#include <map>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>

#include "SRawEvent.h"
#include "SRecEvent.h"

#include "dst/DataStruct.h"

using namespace std;

int main(int argc, char* argv[])
{
    Spill* p_spill = new Spill; Spill& spill = *p_spill;
    TFile* spillFile = new TFile(argv[3]);
    TTree* spillTree = (TTree*)spillFile->Get("save");

    spillTree->SetBranchAddress("spill", &p_spill);

    map<int, Spill> spillBank;
    for(int i = 0; i < spillTree->GetEntries(); ++i)
    {
        spillTree->GetEntry(i);
        if(spill.goodSpill()) spillBank.insert(map<int, Spill>::value_type(spill.spillID, spill));
    }

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    SRawEvent* rawEvent = new SRawEvent();
    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    TFile* saveFile[7];
    TTree* saveTree[7];
    for(int i = 1; i < 8; ++i)
    {
        saveFile[i-1] = new TFile(Form("%s_%d.root", argv[2], i), "recreate");
        saveTree[i-1] = dataTree->CloneTree(0);
    }

    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);

        int spillID = rawEvent->getSpillID();
        int eventID = rawEvent->getEventID();
        if(spillBank.find(spillID) != spillBank.end() && (eventID >= spillBank[spillID].eventID_min && eventID <= spillBank[spillID].eventID_max))
        {
            int index = spillBank[spillID].targetPos - 1;
            if(index >= 0 && index < 7) saveTree[index]->Fill();
        }

        rawEvent->clear();
    }

    for(int i = 0; i < 7; ++i)
    {
        saveFile[i]->cd();
        saveTree[i]->Write();
        saveFile[i]->Close();
    }

    return EXIT_SUCCESS;
}
