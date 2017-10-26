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
#include <TClonesArray.h>
#include <TString.h>

#include "SRecEvent.h"
#include "JobOptsSvc.h"
#include "GeomSvc.h"
#include "MySQLSvc.h"

using namespace std;

int main(int argc, char **argv)
{
    cout << "Uploading file: " << argv[1] << " to sql schema " << argv[2] << endl;

    ///Initialize job option service
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
    p_jobOptsSvc->init(argv[1]);

    ///Initialize the geometry service and output file
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    ///Intialize the mysql service
    MySQLSvc* p_mysqlSvc = MySQLSvc::instance();
    p_mysqlSvc->setUserPasswd("production", "qqbar2mu+mu-");
    p_mysqlSvc->connectOutput(argv[4], atoi(argv[5]));
    p_mysqlSvc->setOutputSchema(argv[3]);

    //Set the ktracked bit in summary table
    if(!p_jobOptsSvc->m_mcMode) p_mysqlSvc->getOutputServer()->Exec(Form("UPDATE summary.production SET ktracked=0,kTrackStart=NOW(),kTrackEnd=NULL WHERE production='%s'", argv[3]));

    ///Retrieve data from file
    TClonesArray* tracklets = new TClonesArray("Tracklet");
    SRecEvent* recEvent = new SRecEvent();

    ///Get trees
    TFile* dataFile = new TFile(argv[2], "READ");
    TTree* configTree = (TTree*)dataFile->Get("config");
    TTree* dataTree = (TTree*)dataFile->Get("save");
    TTree* mixTree = (TTree*)dataFile->Get("save_mix");
    TTree* ppTree = (TTree*)dataFile->Get("save_pp");
    TTree* mmTree = (TTree*)dataFile->Get("save_mm");
    TTree* mixppTree = (TTree*)dataFile->Get("save_mix_pp");
    TTree* mixmmTree = (TTree*)dataFile->Get("save_mix_mm");

    ///Table name and corresponding trees
    TString tableSuffix[6] = {"", "Mix", "PP", "MM", "MixPP", "MixMM"};
    TTree* trees[6] = {dataTree, mixTree, ppTree, mmTree, mixppTree, mixmmTree};

    ///Initialize data tables
    if(!p_mysqlSvc->initWriter()) exit(EXIT_FAILURE);
    if(!p_jobOptsSvc->m_mcMode && !p_mysqlSvc->initBakWriter()) exit(EXIT_FAILURE);

    ///Write the tracker configuration table
    p_mysqlSvc->writeInfoTable(configTree);

    ///Upload all tables
    int uploadListLen = p_jobOptsSvc->m_mcMode ? 1 : 6;
    for(int i = 0; i < uploadListLen; ++i)
    {
        trees[i]->SetBranchAddress("recEvent", &recEvent);

        TClonesArray* trackletArray = NULL;
        if(trees[i]->GetListOfBranches()->Contains("tracklets"))
        {
            trackletArray = tracklets;
            trees[i]->SetBranchAddress("tracklets", &trackletArray);
        }

        p_mysqlSvc->resetWriter();
        int nEvents = trees[i]->GetEntries();
        int printFreq = nEvents/100 > 1 ? nEvents/100 : 1;
        for(int j = 0; j < nEvents; ++j)
        {
            trees[i]->GetEntry(j);
            if(j % printFreq == 0) cout << Form("Uploading %s event, %d percent finished.", tableSuffix[i].Data(), (j+1)*100/nEvents) << endl;

            p_mysqlSvc->writeTrackingRes(tableSuffix[i], recEvent, trackletArray);
        }
        p_mysqlSvc->finalizeWriter();
        cout << Form("Uploaded %s data events successfully", tableSuffix[i].Data()) << endl << endl;
    }
    cout << "sqlResWriter ends successfully." << endl;

    //Set the ktracked bit in summary table
    if(!p_jobOptsSvc->m_mcMode) p_mysqlSvc->getOutputServer()->Exec(Form("UPDATE summary.production SET ktracked=1,kTrackEnd=NOW() WHERE production='%s'", argv[3]));

    delete p_mysqlSvc;
    delete p_geomSvc;
    delete p_jobOptsSvc;

    return EXIT_SUCCESS;
}
