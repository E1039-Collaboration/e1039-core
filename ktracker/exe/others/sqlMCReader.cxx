#include <iostream>
#include <string>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TLorentzVector.h>

#include "SRawEvent.h"
#include "GeomSvc.h"
#include "MySQLSvc.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char **argv)
{
    cout << "Exporting Run: " << argv[1] << " to ROOT file: " << argv[2] << endl;

    ///Initialize the default job options
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();

    //Initialization
    TString inputSchema;
    TString outputFile;
    TString inputServer;
    int inputPort;
    int nEvents;
    int startEvent;

    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    if(argc == 2)    //initialization via opts file
    {
        jobOptsSvc->init(argv[1]);

        inputSchema = jobOptsSvc->m_inputSchema;
        outputFile = jobOptsSvc->m_outputFile;
        inputServer = jobOptsSvc->m_mySQLInputServer;
        inputPort = jobOptsSvc->m_mySQLInputPort;

        nEvents = jobOptsSvc->m_nEvents;
        startEvent = jobOptsSvc->m_firstEvent;
    }
    else if(argc <= 8)
    {
        jobOptsSvc->init(argv[1]);

        inputSchema = argv[2];
        outputFile = argv[3];
        inputServer = argv[4];
        inputPort = atoi(argv[5]);
        nEvents = argc > 6 ? atoi(argv[6]) : -1;
        startEvent = argc > 6 ? atoi(argv[7]) : 0;
    }
    else
    {
        cout << "Usage: " << argv[0] << "  <options file>" << endl;
        exit(EXIT_FAILURE);
    }

    ///Initialize the geometry service and output file
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
    p_mysqlSvc->connectInput(inputServer.Data(), inputPort);
    p_mysqlSvc->setInputSchema(inputSchema.Data());
    if(!p_mysqlSvc->initReader()) exit(EXIT_FAILURE);

    SRawMCEvent* rawEvent = new SRawMCEvent();

    TFile *saveFile = new TFile(outputFile.Data(), "recreate");
    TTree *saveTree = new TTree("save", "save");

    saveTree->Branch("rawEvent", &rawEvent, 256000, 99);

    int nEventsTotal = p_mysqlSvc->getNEventsFast(nEvents + startEvent, startEvent);
    cout << "Totally " << nEventsTotal << " events in this run to be read" << endl;
    for(int i = 0; i < nEventsTotal; ++i)
    {
        if(!p_mysqlSvc->getNextEvent(rawEvent)) continue;
        cout << "\r Converting event " << rawEvent->getEventID() << ", " << (i+11)*100/nEventsTotal << "% finished." << flush;

        saveTree->Fill();
    }
    cout << endl;
    cout << "sqlMCReader ends successfully." << endl;

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete p_mysqlSvc;
    delete p_geomSvc;;
    delete p_jobOptsSvc;

    return EXIT_SUCCESS;
}
