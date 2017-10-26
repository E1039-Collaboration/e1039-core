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
#include <TStopwatch.h>
#include <TTimeStamp.h>
#include <TString.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "SRecEvent.h"
#include "FastTracklet.h"
#include "KalmanFastTracking.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "EventReducer.h"
#include "JobOptsSvc.h"
#include "GlobalConsts.h"

using namespace std;

int main(int argc, char* argv[])
{
    //Initialization
    TString optfile;
    TString inputFile;
    TString outputFile;
    int nEvents;
    int startEvent;

    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    if(argc == 2)    //initialization via opts file
    {
        optfile = argv[1];
        jobOptsSvc->init(optfile.Data());

        inputFile = jobOptsSvc->m_inputFile;
        outputFile = jobOptsSvc->m_outputFile;
        nEvents = jobOptsSvc->m_nEvents;
        startEvent = jobOptsSvc->m_firstEvent;
    }
    else if(argc <= 5)
    {
        jobOptsSvc->init();

        inputFile = argv[1];
        outputFile = argv[2];
        nEvents = argc >= 4 ? atoi(argv[3]) : -1;
        startEvent = argc == 5 ? atoi(argv[4]) : 0;
    }
    else
    {
        cout << "Usage: " << argv[0] << "  <options file>" << endl;
        exit(EXIT_FAILURE);
    }

    //Initialize the Geometry service
    GeomSvc::instance()->init();

    //Retrieve the raw event
    SRawEvent* rawEvent = jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());
    SRawEvent* orgEvent = jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());

    TFile* dataFile = new TFile(inputFile.Data(), "READ");
    TTree* dataTree = (TTree *)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);

    //Output definition
    int nTracklets;
    TClonesArray* tracklets = new TClonesArray("Tracklet");
    TClonesArray& arr_tracklets = *tracklets;

    double time;
    SRecEvent* recEvent = new SRecEvent();

    TFile* saveFile = new TFile(outputFile.Data(), "recreate");
    jobOptsSvc->save(saveFile);

    TTree* saveTree = new TTree("save", "save");
    saveTree->Branch("rawEvent", &rawEvent, 256000, 99);
    saveTree->Branch("orgEvent", &orgEvent, 256000, 99);
    saveTree->Branch("recEvent", &recEvent, 256000, 99);
    saveTree->Branch("time", &time, "time/D");
    saveTree->Branch("nTracklets", &nTracklets, "nTracklets/I");
    saveTree->Branch("tracklets", &tracklets, 256000, 99);
    tracklets->BypassStreamer();

    //Initialize track finder
    LogInfo("Initializing the track finder and kalman filter ... ");
#ifdef _ENABLE_KF
    KalmanFilter* filter = new KalmanFilter();
    KalmanFastTracking* fastfinder = new KalmanFastTracking();
#else
    KalmanFastTracking* fastfinder = new KalmanFastTracking(false);
#endif

    //Initialize event reducer
    TString opt = "aoc";      //turn on after pulse removal, out of time removal, and cluster removal
    if(jobOptsSvc->m_enableTriggerMask) opt = opt + "t";
    if(jobOptsSvc->m_sagittaReducer) opt = opt + "s";
    if(jobOptsSvc->m_updateAlignment) opt = opt + "e";
    if(jobOptsSvc->m_hodomask) opt = opt + "h";
    if(jobOptsSvc->m_mergeHodo) opt = opt + "m";
    if(jobOptsSvc->m_realization) opt = opt + "r";
    EventReducer* eventReducer = new EventReducer(opt);
    EventReducer* orgEvReducer = new EventReducer("o");

    TStopwatch timer;
    TTimeStamp ts;

    const int offset = startEvent;
    int nEvtMax = nEvents > 0 ? nEvents + offset : dataTree->GetEntries();
    if(nEvtMax > dataTree->GetEntries()) nEvtMax = dataTree->GetEntries();
    LogInfo(ts.AsString() << ": Running from event " << offset << " through to event " << nEvtMax);

    const int printFreq = (nEvtMax - offset)/100 > 1 ? (nEvtMax - offset)/100 : 1;
    for(int i = offset; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);

        const double fracDone = (i - offset + 1)*100/(nEvtMax - offset);
        if(0 == i % printFreq)
        {
            timer.Stop();
            ts.Set(); cout << Form("%s: Converting Event %d, %.02f%% finished.  Time to process last %d events shown below:", ts.AsString(), rawEvent->getEventID(), fracDone, printFreq) << endl;
            timer.Print();
            timer.Start();
        }

        clock_t time_single = clock();

        *orgEvent = *rawEvent;
        eventReducer->reduceEvent(rawEvent);
        orgEvReducer->reduceEvent(orgEvent);
        recEvent->setRecStatus(fastfinder->setRawEvent(rawEvent));

        //Fill the TClonesArray
        arr_tracklets.Clear();
        std::list<Tracklet>& rec_tracklets = fastfinder->getFinalTracklets();

        nTracklets = 0;
        recEvent->setRawEvent(rawEvent);
        for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
        {
            iter->calcChisq();
            //iter->print();
            new(arr_tracklets[nTracklets]) Tracklet(*iter);
            ++nTracklets;

#ifndef _ENABLE_KF
            SRecTrack recTrack = iter->getSRecTrack();
            recEvent->insertTrack(recTrack);
#endif
        }

#ifdef _ENABLE_KF
        std::list<SRecTrack>& rec_tracks = fastfinder->getSRecTracks();
        for(std::list<SRecTrack>::iterator iter = rec_tracks.begin(); iter != rec_tracks.end(); ++iter)
        {
            //iter->print();
            recEvent->insertTrack(*iter);
        }
#endif

        if(jobOptsSvc->m_enableEvaluation)
        {
            for(int j = 3; j != 0; --j)
            {
                std::list<Tracklet>& tracklets_temp = fastfinder->getTrackletList(j);
                for(std::list<Tracklet>::iterator iter = tracklets_temp.begin(); iter != tracklets_temp.end(); ++iter)
                {
                    iter->calcChisq();
                    
                    new(arr_tracklets[nTracklets]) Tracklet(*iter);
                    ++nTracklets;
                }
            }
        }

        // time of each event
        time_single = clock() - time_single;
        time = double(time_single)/CLOCKS_PER_SEC;

        // sort the track list, and empty the large hit list in rawEvent if needed
        //recEvent->reIndex();
        if(!jobOptsSvc->m_attachRawEvent)
        {
            rawEvent->empty();
            orgEvent->empty();
        }

        saveTree->Fill();
        if(saveTree->GetEntries() % 1000 == 0) saveTree->AutoSave("SaveSelf");

        recEvent->clear();
        rawEvent->clear();
        orgEvent->clear();
    }
    saveTree->AutoSave("SaveSelf");
    cout << endl;
    ts.Set(); cout << ts.AsString() << ": kFastTracking ends successfully." << endl;

    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    delete fastfinder;
    //delete eventReducer;
#ifdef _ENABLE_KF
    filter->close();
#endif

    return EXIT_SUCCESS;
}
