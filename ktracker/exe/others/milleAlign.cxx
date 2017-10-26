#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include <time.h>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>
#include <TMatrixD.h>
#include <TLorentzVector.h>
#include <TClonesArray.h>
#include <TMath.h>
#include <TF1.h>
#include <TH1D.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"
#include "SMillepede.h"
#include "SMillepedeUtil.h"
#include "JobOptsSvc.h"

#include "GlobalConsts.h"

int main(int argc, char *argv[])
{
    //Initialize default job options
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();

    //Initialize geometry service
    GeomSvc* geometrySvc = GeomSvc::instance();
    geometrySvc->init();

    //Retrieve the raw event
    LogInfo("Retrieving the event stored in ROOT file ... ");
    SRawEvent* rawEvent = jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());
#ifdef _ENABLE_KF
    SRecEvent* recEvent = new SRecEvent();
#else
    TClonesArray* tracklets = new TClonesArray("Tracklet");
    tracklets->Clear();
#endif

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree *)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
#ifdef _ENABLE_KF
    dataTree->SetBranchAddress("recEvent", &recEvent);
#else
    dataTree->SetBranchAddress("tracklets", &tracklets);
#endif

    //Initialize track finder
    LogInfo("Initializing the millepede ... ");
    SMillepede* mille = new SMillepede();
    mille->init("mille.conf");
    if(argc > 4)
    {
        mille->bookEvaluationTree(argv[4]);
    }
    else
    {
        mille->bookEvaluationTree("align_eval.root");
    }

    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; i++)
    {
        dataTree->GetEntry(i);

#ifdef _ENABLE_KF
        mille->setEvent(rawEvent, recEvent);
#else
        mille->setEvent(tracklets);
        tracklets->Clear();
#endif
    }

    mille->fitAlignment();
    mille->printQAPlots();
    mille->printResults(argv[2], argv[3]);

    delete mille;

    return EXIT_SUCCESS;
}
