#include <iostream>
#include <vector>
#include <list>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TVector3.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "TriggerRoad.h"
#include "TriggerAnalyzer.h"

using namespace std;

int main(int argc, char* argv[])
{
    ///Initialize the default job options
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
    p_jobOptsSvc->init();
    p_jobOptsSvc->m_triggerL1 = atoi(argv[3]);

    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    TriggerAnalyzer* triggerAna = new TriggerAnalyzer();
    triggerAna->init();
    triggerAna->buildTriggerTree();

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    SRawEvent* rawEvent = p_jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());
    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = dataTree->CloneTree(0);

    int nEvents = dataTree->GetEntries();
    for(int i = 0; i < nEvents; ++i)
    {
        dataTree->GetEntry(i);
        cout << "\r Processing event " << rawEvent->getEventID() << ", " << i*100/nEvents << "% finished." << flush;

        rawEvent->setTriggerEmu(triggerAna->acceptEvent(rawEvent, USE_TRIGGER_HIT));
        list<TriggerRoad>& p_roads_found = triggerAna->getRoadsFound(+1);
        list<TriggerRoad>& m_roads_found = triggerAna->getRoadsFound(-1);

        int nRoads[4] = {triggerAna->getNRoadsPosTop(), triggerAna->getNRoadsPosBot(), triggerAna->getNRoadsNegTop(), triggerAna->getNRoadsNegBot()};
        rawEvent->setNRoads(nRoads);

        saveTree->Fill();
        rawEvent->clear();
    }
    cout << endl;
    cout << "triggerAna ended successfully." << endl;

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete triggerAna;
    return EXIT_SUCCESS;
}
