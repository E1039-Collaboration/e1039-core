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
#include "TriggerRoad.h"
#include "TriggerAnalyzer.h"
#include "EventReducer.h"
#include "JobOptsSvc.h"

#include "dst/DataStruct.h"

using namespace std;
typedef pair<int, int> eventPair;

void generateEventIDPairs(int nSig, int nBkg, vector<eventPair>& eventIDs)
{
    int nMax = nSig > nBkg ? nSig : nBkg;
    int nMin = nSig > nBkg ? nBkg : nSig;
    cout << nMin << "  " << nSig << "  " << nBkg << endl;

    TRandom3 rndm(0);
    set<int> pool;
    while(pool.size() < nMin)
    {
        pool.insert(int(rndm.Rndm()*nMax));
    }

    //prepare the event list, very important to keep everything in acsending order
    //note std::set is automatically sorted
    eventIDs.clear();
    int index = 0;
    for(set<int>::iterator iter = pool.begin(); iter != pool.end(); ++iter)
    {
        if(nSig >= nBkg)
        {
            eventIDs.push_back(make_pair(*iter, index));
        }
        else
        {
            eventIDs.push_back(make_pair(index, *iter));
        }
        ++index;
    }
}

int main(int argc, char *argv[])
{
    //Initialize job options
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    jobOptsSvc->init(argv[1]);

    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    TriggerAnalyzer* triggerAna = new TriggerAnalyzer();
    triggerAna->init();
    triggerAna->buildTriggerTree();

    //Load spill info
    map<int, Spill> spillBank;
    if(argc > 6) //spill info are provided
    {
        Spill* p_spill = new Spill; Spill& spill = *p_spill;
        TFile* spillFile = new TFile(argv[6]);
        TTree* spillTree = (TTree*)spillFile->Get("save");

        spillTree->SetBranchAddress("spill", &p_spill);
        for(int i = 0; i < spillTree->GetEntries(); ++i)
        {
            spillTree->GetEntry(i);
            if(spill.goodSpill()) spillBank.insert(map<int, Spill>::value_type(spill.spillID, spill));
        }

        cout << "Loaded " << spillBank.size() << " spills from " << argv[6] << endl;
    }

    //Event pre-processor
    EventReducer* reducer1 = new EventReducer("r");  //for MC
    EventReducer* reducer2 = new EventReducer("e");  //for Bkg

    TFile* mcFile = new TFile(argv[2], "READ");
    TTree* mcTree = (TTree*)mcFile->Get("save");

    SRawMCEvent* mcEvent = new SRawMCEvent();
    mcTree->SetBranchAddress("rawEvent", &mcEvent);

    TFile* bkgFile = new TFile(argv[3], "READ");
    TTree* bkgTree = (TTree*)bkgFile->Get("save");

    SRawEvent* bkgEvent = new SRawEvent();
    bkgTree->SetBranchAddress("rawEvent", &bkgEvent);

    //generate a random set of eventID pairs
    vector<eventPair> eventIDs;
    generateEventIDPairs(mcTree->GetEntries(), bkgTree->GetEntries(), eventIDs);

    //clean output
    TFile* saveFile1 = new TFile(argv[4], "recreate");
    TTree* saveTree1 = mcTree->CloneTree(0);

    //messy output
    TFile* saveFile2 = new TFile(argv[5], "recreate");
    TTree* saveTree2 = new TTree("save", "save");

    SRawMCEvent* rawEvent = new SRawMCEvent();
    saveTree2->Branch("rawEvent", &rawEvent, 256000, 99);

    for(int i = 0; i < eventIDs.size(); ++i)
    {
        mcTree->GetEntry(eventIDs[i].first);
        bkgTree->GetEntry(eventIDs[i].second);
        if(i % 1000 == 0) cout << i << " " << eventIDs[i].first << " " << eventIDs[i].second << endl;

        //set the intensity info to MC first
        mcEvent->setEventInfo(bkgEvent);

        //strip MC events to simulate efficiency, update the alignment parameters for the bkg events
        if(argc > 6) reducer1->setChamEff(0.94*(1. - 0.1*mcEvent->getIntensity()*spillBank[bkgEvent->getSpillID()].QIEUnit()));
        reducer1->reduceEvent(mcEvent);
        reducer2->reduceEvent(bkgEvent);

        //merge the bkg events with MC
        *rawEvent = *mcEvent;
        rawEvent->mergeEvent(*bkgEvent);
        rawEvent->setTriggerEmu(triggerAna->acceptEvent(rawEvent, USE_TRIGGER_HIT));
        int nRoads1[4] = {triggerAna->getNRoadsPosTop(), triggerAna->getNRoadsPosBot(), triggerAna->getNRoadsNegTop(), triggerAna->getNRoadsNegBot()};
        rawEvent->setNRoads(nRoads1);
        rawEvent->setEventInfo(bkgEvent->getRunID(), bkgEvent->getSpillID(), bkgEvent->getEventID());
        saveTree2->Fill();

        //re-update the MC event trigger info, as it's contaminated by the bkg event info
        mcEvent->setTriggerEmu(triggerAna->acceptEvent(mcEvent, USE_TRIGGER_HIT));
        int nRoads2[4] = {triggerAna->getNRoadsPosTop(), triggerAna->getNRoadsPosBot(), triggerAna->getNRoadsNegTop(), triggerAna->getNRoadsNegBot()};
        mcEvent->setNRoads(nRoads2);

        saveTree1->Fill();

        rawEvent->clear();
        mcEvent->clear();
        bkgEvent->clear();
    }

    saveFile1->cd();
    saveTree1->Write();
    saveFile1->Close();

    saveFile2->cd();
    saveTree2->Write();
    saveFile2->Close();

    delete triggerAna;
    return EXIT_SUCCESS;
}
