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

#include "MySQLSvc.h"
#include "GeomSvc.h"
#include "SRawEvent.h"
#include "TriggerRoad.h"
#include "TriggerAnalyzer.h"

using namespace std;

int main(int argc, char* argv[])
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
    p_mysqlSvc->connect();
    p_mysqlSvc->setWorkingSchema(argv[1]);

    TriggerAnalyzer* triggerAna = new TriggerAnalyzer();
    triggerAna->init();
    triggerAna->buildTriggerTree();

    SRawEvent* rawEvent = new SRawEvent();

    int H1XT, H2XT, H3XT, H4XT;
    int H1XB, H2XB, H3XB, H4XB;

    int H1YT, H2YT, H3YT, H4YT;
    int H1YB, H2YB, H3YB, H4YB;

    int p_FPGA, m_FPGA;
    int p_roadID[10000];
    int m_roadID[10000];

    int NIMXT, NIMXB;
    int NIMYT, NIMYB;

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("rawEvent", &rawEvent, 256000, 99);

    saveTree->Branch("H1XT", &H1XT, "H1XT/I");
    saveTree->Branch("H2XT", &H2XT, "H2XT/I");
    saveTree->Branch("H3XT", &H3XT, "H3XT/I");
    saveTree->Branch("H4XT", &H4XT, "H4XT/I");
    saveTree->Branch("H1XB", &H1XB, "H1XB/I");
    saveTree->Branch("H2XB", &H2XB, "H2XB/I");
    saveTree->Branch("H3XB", &H3XB, "H3XB/I");
    saveTree->Branch("H4XB", &H4XB, "H4XB/I");

    saveTree->Branch("H1YT", &H1YT, "H1YT/I");
    saveTree->Branch("H2YT", &H2YT, "H2YT/I");
    saveTree->Branch("H3YT", &H3YT, "H3YT/I");
    saveTree->Branch("H4YT", &H4YT, "H4YT/I");
    saveTree->Branch("H1YB", &H1YB, "H1YB/I");
    saveTree->Branch("H2YB", &H2YB, "H2YB/I");
    saveTree->Branch("H3YB", &H3YB, "H3YB/I");
    saveTree->Branch("H4YB", &H4YB, "H4YB/I");

    saveTree->Branch("p_FPGA", &p_FPGA, "p_FPGA/I");
    saveTree->Branch("p_roadID", p_roadID, "p_roadID[p_FPGA]/I");
    saveTree->Branch("m_FPGA", &m_FPGA, "m_FPGA/I");
    saveTree->Branch("m_roadID", m_roadID, "m_roadID[m_FPGA]/I");

    saveTree->Branch("NIMXT", &NIMXT, "NIMXT/I");
    saveTree->Branch("NIMXB", &NIMXB, "NIMXB/I");
    saveTree->Branch("NIMYT", &NIMYT, "NIMYT/I");
    saveTree->Branch("NIMYB", &NIMYB, "NIMYB/I");

    int nEvents = p_mysqlSvc->getNEvents();
    for(int i = 0; i < nEvents; ++i)
    {
        if(!p_mysqlSvc->getNextEvent(rawEvent)) continue;
        cout << "\r Processing event " << rawEvent->getEventID() << ", " << i*100/nEvents << "% finished." << flush;

        //Init
        H1XT = 0;
        H2XT = 0;
        H3XT = 0;
        H4XT = 0;
        H1XB = 0;
        H2XB = 0;
        H3XB = 0;
        H4XB = 0;
        H1YT = 0;
        H2YT = 0;
        H3YT = 0;
        H4YT = 0;
        H1YB = 0;
        H2YB = 0;
        H3YB = 0;
        H4YB = 0;
        p_FPGA = 0;
        m_FPGA = 0;

        //NIM trigger part
        vector<Hit> triggerHits = rawEvent->getTriggerHits();
        for(vector<Hit>::iterator hit = triggerHits.begin(); hit != triggerHits.end(); ++hit)
        {
            int detectorID = hit->detectorID;
            int elementID = hit->elementID;

            if(detectorID == 25) ++H1XB;
            if(detectorID == 31) ++H2XB;
            if(detectorID == 33) ++H3XB;
            if(detectorID == 39) ++H4XB;
            if(detectorID == 26) ++H1XT;
            if(detectorID == 32) ++H2XT;
            if(detectorID == 34) ++H3XT;
            if(detectorID == 40) ++H4XT;

            if((detectorID == 27 || detectorID == 28) && elementID > 10) ++H1YT;
            if((detectorID == 29 || detectorID == 30) && elementID > 10) ++H2YT;
            if((detectorID == 35 || detectorID == 36) && elementID > 8) ++H3YT;
            if((detectorID == 37 || detectorID == 38) && elementID > 8) ++H4YT;
            if((detectorID == 27 || detectorID == 28) && elementID <= 10) ++H1YB;
            if((detectorID == 29 || detectorID == 30) && elementID < 10) ++H2YB;
            if((detectorID == 35 || detectorID == 36) && elementID <= 8) ++H3YB;
            if((detectorID == 37 || detectorID == 38) && elementID <= 8) ++H4YB;
        }
        NIMXT = H1XT > 0 && H2XT > 0 && H3XT > 0 && H4XT > 0 ? 1 : 0;
        NIMXB = H1XB > 0 && H2XB > 0 && H3XB > 0 && H4XB > 0 ? 1 : 0;
        NIMYT = H1YT > 0 && H2YT > 0 && H3YT > 0 && H4YT > 0 ? 1 : 0;
        NIMYB = H1YB > 0 && H2YB > 0 && H3YB > 0 && H4YB > 0 ? 1 : 0;

        triggerAna->acceptEvent(rawEvent);
        list<TriggerRoad>& p_roads_found = triggerAna->getRoadsFound(+1);
        list<TriggerRoad>& m_roads_found = triggerAna->getRoadsFound(-1);
        p_FPGA = 0;;
        for(list<TriggerRoad>::iterator iter = p_roads_found.begin(); iter != p_roads_found.end(); ++iter)
        {
            p_roadID[p_FPGA++] = iter->roadID;
        }
        m_FPGA = 0;;
        for(list<TriggerRoad>::iterator iter = m_roads_found.begin(); iter != m_roads_found.end(); ++iter)
        {
            m_roadID[m_FPGA++] = iter->roadID;
        }

        saveTree->Fill();
    }
    cout << endl;
    cout << "triggerAna ended successfully." << endl;

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete triggerAna;

    return EXIT_SUCCESS;
}
