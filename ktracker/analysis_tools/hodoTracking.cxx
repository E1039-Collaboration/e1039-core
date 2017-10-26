#include <iostream>
#include <vector>
#include <list>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TVector3.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "TriggerRoad.h"
#include "TriggerAnalyzer.h"

using namespace std;

int main(int argc, char* argv[])
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    SRawEvent* rawEvent = new SRawEvent();

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    TriggerAnalyzer* triggerAna = new TriggerAnalyzer();
    triggerAna->init("roads_DY.root", 1.E-3, 1.E5);
    triggerAna->buildTriggerTree();

    Int_t nFired1;
    Double_t pT1[1000];

    Int_t nFired2;
    Double_t pT2[1000];

    Int_t nAll;
    Double_t mass[100000];

    Int_t flag;
    Double_t p_mass, p_pT1, p_pT2;

    SRawEvent* rawEvent_new = new SRawEvent();

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = dataTree->CloneTree(0);

    saveTree->Branch("rawEvent_new", &rawEvent_new, 256000, 99);

    saveTree->Branch("nFired1", &nFired1, "nFired1/I");
    saveTree->Branch("pT1", pT1, "pT1[nFired1]/D");

    saveTree->Branch("nFired2", &nFired2, "nFired2/I");
    saveTree->Branch("pT2", pT2, "pT2[nFired2]/D");

    saveTree->Branch("nAll", &nAll, "nAll/I");
    saveTree->Branch("mass_rec", mass, "mass_rec[nAll]/D");

    saveTree->Branch("flag", &flag, "flag/I");
    saveTree->Branch("p_mass", &p_mass, "p_mass/D");
    saveTree->Branch("p_pT1", &p_pT1, "p_pT1/D");
    saveTree->Branch("p_pT2", &p_pT2, "p_pT2/D");

    for(int i = 0; i < dataTree->GetEntries(); i++)
    {
        dataTree->GetEntry(i);

        rawEvent->reIndex("a");
        rawEvent_new->setEventInfo(rawEvent->getRunID(), rawEvent->getSpillID(), rawEvent->getEventID());

        flag = triggerAna->acceptEvent(rawEvent) ? 1 : -1;
        std::list<TriggerRoad>& p_roads_found = triggerAna->getRoadsFound(+1);
        std::list<TriggerRoad>& m_roads_found = triggerAna->getRoadsFound(-1);

        nFired1 = 0;
        for(std::list<TriggerRoad>::iterator iter = p_roads_found.begin(); iter != p_roads_found.end(); ++iter)
        {
            pT1[nFired1++] = iter->pT_mean;
            for(int j = 0; j < 4; ++j)
            {
                rawEvent_new->insertHit(rawEvent->getHit(iter->detectorIDs[j], iter->elementIDs[j]));
            }
        }

        nFired2 = 0;
        for(std::list<TriggerRoad>::iterator iter = m_roads_found.begin(); iter != m_roads_found.end(); ++iter)
        {
            pT2[nFired2++] = iter->pT_mean;
            for(int j = 0; j < 4; ++j)
            {
                rawEvent_new->insertHit(rawEvent->getHit(iter->detectorIDs[j], iter->elementIDs[j]));
            }
        }

        if((nFired1 > 50 || nFired2 > 50) || (nFired1 == 0 && nFired2 == 0))
        {
            rawEvent->clear();
            rawEvent_new->clear();

            continue;
        }

        p_roads_found.sort(TriggerRoad::byWeight);
        m_roads_found.sort(TriggerRoad::byWeight);

        nAll = 0;
        for(std::list<TriggerRoad>::iterator iter = p_roads_found.begin(); iter != p_roads_found.end(); ++iter)
        {
            for(std::list<TriggerRoad>::iterator jter = m_roads_found.begin(); jter != m_roads_found.end(); ++jter)
            {
                mass[nAll++] = iter->pT_mean + jter->pT_mean;
            }
        }

        if(nAll == 0)
        {
            p_mass = -1.;
            p_pT1 = -1.;
            p_pT2 = -1.;
        }
        else
        {
            p_pT1 = p_roads_found.front().pT_mean;
            p_pT2 = m_roads_found.front().pT_mean;
            p_mass = p_pT1 + p_pT2;
        }

        rawEvent_new->reIndex("a");
        saveTree->Fill();
        rawEvent->clear();
        rawEvent_new->clear();
    }

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete triggerAna;

    return EXIT_SUCCESS;
}
