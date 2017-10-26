#include <iostream>
#include <iomanip>
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
#include <TStopwatch.h>

#include "SRawEvent.h"
#include "GeomSvc.h"
#include "MySQLSvc.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char **argv)
{
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();

    //parse options in both formats
    string inputSchema, outputFilename;
    int nEventsRequested = -1;

    //if only one arg is give, use the options files
    if(argc == 2)
    {
        //Initialize job options
        p_jobOptsSvc->init(argv[1]);
        inputSchema = p_jobOptsSvc->m_inputSchema;
        outputFilename = p_jobOptsSvc->m_inputFile;
        nEventsRequested = p_jobOptsSvc->m_nEvents;
    }
    else if(argc == 3)
    {
        p_jobOptsSvc->init();
        inputSchema = argv[1];
        outputFilename = argv[2];
    }
    else if(argc == 4)
    {
        p_jobOptsSvc->init();
        inputSchema = argv[1];
        outputFilename = argv[2];
        nEventsRequested = atoi(argv[3]);
    }
    else
    {
        cout << "Usage: " << argv[0] << "  <options file>" << endl;
        cout << "  ---  OR ---" << endl;
        cout << "     : " << argv[1] << " <input schema> <output file> [nEvents]" << endl;
        exit(0);
    }
    cout << "Exporting Run: " << inputSchema << " to ROOT file: " << outputFilename << endl;

    TStopwatch timer;
    timer.Start();

    ///Initialize the geometry service
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    ///Initialize the mysql service
    MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
    p_mysqlSvc->connectInput();
    p_mysqlSvc->setInputSchema(inputSchema.c_str());
    if(!p_mysqlSvc->initReader()) exit(EXIT_FAILURE);

    ///Definition of the output structure
    SRawEvent* rawEvent = new SRawEvent();

    TFile* saveFile = new TFile(outputFilename.c_str(), "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("rawEvent", &rawEvent, 256000, 99);

    //will force save every 1000 events
    int saveFreq = 1000;

    int nEvents = p_mysqlSvc->getNEventsFast();
    cout << "Totally " << nEvents << " events in this run" << endl;

    //if user specified number of events, then do that many
    //   but make sure they can't request more than exist
    if(nEventsRequested>0) nEvents = std::min( nEvents, nEventsRequested );
    cout << "Will process " << nEvents << " events" << endl;

    //plan to print progress at each %1 (unless in live mode)
    const int printFreq = (nEvents/100);

    timer.Start();
    for(int i = 0; i < nEvents; ++i)
    {
        if(!p_mysqlSvc->getNextEvent(rawEvent)) continue;

        //print progress every event or every 1%
        const int fracDone = (i+1)*100/nEvents;
        if(i % printFreq == 0)
        {
            timer.Stop();
            cout << Form("Converting Event %d, %d%% finished.  Time to process last %d events shown below:", rawEvent->getEventID(), fracDone, printFreq) << endl;
            timer.Print();
            timer.Start();
        }

        saveTree->Fill();

        //save every n events to avoid large losses
        if(i % saveFreq == 0) saveTree->AutoSave("SaveSelf");
    }
    cout << endl;
    cout << "sqlDataReader ends successfully." << endl;

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete p_mysqlSvc;
    delete p_geomSvc;
    delete p_jobOptsSvc;

    return EXIT_SUCCESS;
}
