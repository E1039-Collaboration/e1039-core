#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "TriggerAnalyzer.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char *argv[])
{
    //Init the job options
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
    p_jobOptsSvc->init();

    //parse the options
    TString options(argv[1]);
    options.ToLower();
    bool alignment = options.Contains("a");
    bool calibration = options.Contains("c");
    bool trigger = options.Contains("t");
    bool replace = options.Contains("r");

    if(alignment)   cout << " Updator: updating the alignment parameters." << endl;
    if(calibration) cout << " Updator: updating the calibration parameters." << endl;
    if(trigger)     cout << " Updator: updating the trigger emulation." << endl;
    if(replace)     cout << " Updator: will overwrite the original file." << endl;

    //Initialize geometry service
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    //Initialize the trigger analyzer
    TriggerAnalyzer* p_triggerAna = new TriggerAnalyzer();
    p_triggerAna->init();
    p_triggerAna->buildTriggerTree();

    //Input files
    SRawEvent* rawEvent = p_jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());

    TFile *dataFile = new TFile(argv[2], "READ");
    TTree *dataTree = (TTree *)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    //Output files
    TFile *saveFile = new TFile(argv[3], "recreate");
    TTree *saveTree = dataTree->CloneTree(0);

    //Do the updating work
    for(int i = 0; i < dataTree->GetEntries(); i++)
    {
        dataTree->GetEntry(i);

        //apply the trigger road info
        if(trigger)
        {
            rawEvent->setTriggerEmu(p_triggerAna->acceptEvent(rawEvent));

            int nRoads[4] = {p_triggerAna->getNRoadsPosTop(), p_triggerAna->getNRoadsPosBot(), p_triggerAna->getNRoadsNegTop(), p_triggerAna->getNRoadsNegBot()};
            rawEvent->setNRoads(nRoads);
        }

        //apply hit-level alignment/calibration
        if(alignment || calibration)
        {
            for(unsigned int j = 0; j < rawEvent->getNHitsAll(); ++j)
            {
                Hit h = rawEvent->getHit(j);

                if(alignment) h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);
                if(calibration && (h.detectorID <= 30 || h.detectorID >= 47) && h.isInTime() && p_geomSvc->isCalibrationLoaded())
                {
                    h.driftDistance = p_geomSvc->getDriftDistance(h.detectorID, h.tdcTime);
                }

                rawEvent->setHit(j, h);
            }

            for(unsigned int j = 0; j < rawEvent->getNTriggerHits(); ++j)
            {
                Hit h = rawEvent->getTriggerHit(j);
                if(alignment) h.pos = p_geomSvc->getMeasurement(h.detectorID, h.elementID);

                rawEvent->setTriggerHit(j, h);
            }
        }

        saveTree->Fill();
        rawEvent->clear();
    }

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    if(replace)
    {
        char cmd[300];
        sprintf(cmd, "mv %s %s", argv[3], argv[2]);
        cout << cmd << endl;
        system(cmd);
    }

    return 1;
}
