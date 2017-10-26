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
#include <TRandom.h>
#include <TString.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"
#include "JobOptsSvc.h"
#include "FastTracklet.h"
#include "MySQLSvc.h"

#include "GlobalConsts.h"

using namespace std;

int main(int argc, char *argv[])
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

    //Initialize the geometry service
    GeomSvc::instance()->init();

    //Retrieve the raw event
    SRawEvent* rawEvent = jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());
    SRawEvent* orgEvent = jobOptsSvc->m_mcMode ? (new SRawMCEvent()) : (new SRawEvent());
    SRecEvent* recEvent = new SRecEvent();
    TClonesArray* tracklets = new TClonesArray("Tracklet");

    TFile* dataFile = new TFile(inputFile.Data(), "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    if(dataTree->FindBranch("orgEvent") != NULL) dataTree->SetBranchAddress("orgEvent", &orgEvent);   //backward compatibility
    dataTree->SetBranchAddress("recEvent", &recEvent);
    dataTree->SetBranchAddress("tracklets", &tracklets);

    TFile* saveFile = new TFile(outputFile.Data(), "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("rawEvent", &rawEvent, 256000, 99);
    saveTree->Branch("orgEvent", &orgEvent, 256000, 99);
    saveTree->Branch("recEvent", &recEvent, 256000, 99);
    saveTree->Branch("tracklets", &tracklets, 256000, 99);
    tracklets->BypassStreamer();

    // save if the configuration file if exists from tracker output
    if(dataFile->GetListOfKeys()->Contains("config"))
    {
        TTree* configTree = (TTree*)dataFile->Get("config");

        saveFile->cd();
        configTree->CloneTree()->Write();
    }
    jobOptsSvc->save(saveFile, "vconfig");

    //Initialize track finder
    LogInfo("Initializing the track finder and kalman filter ... ");
    VertexFit* vtxfit = new VertexFit();
    vtxfit->enableOptimization();
    if(jobOptsSvc->m_enableEvaluation)
    {
        string evalFileName = "eval_" + jobOptsSvc->m_outputFile;
        vtxfit->bookEvaluation(evalFileName.c_str());
    }

    const int offset = startEvent;
    int nEvtMax = nEvents > 0 ? nEvents + offset : dataTree->GetEntries();
    if(nEvtMax > dataTree->GetEntries()) nEvtMax = dataTree->GetEntries();
    LogInfo("Running from event " << offset << " through to event " << nEvtMax);
    for(int i = offset; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        if(i % 1000 == 0)
        {
            cout << "\r Processing event " << i << " with eventID = " << recEvent->getEventID() << ", ";
            cout << (i + 1)*100/nEvtMax << "% finished .. " << flush;
        }

        recEvent->setRecStatus(vtxfit->setRecEvent(recEvent));

        saveTree->Fill();
        if(saveTree->GetEntries() % 1000 == 0) saveTree->AutoSave("SaveSelf");

        recEvent->clear();
        tracklets->Clear();
        if(jobOptsSvc->m_attachRawEvent) rawEvent->clear();
    }
    cout << endl;
    cout << "kVertex ends successfully." << endl;

    saveFile->cd();
    saveTree->Write();

    //If it's for MC then end now
    if(jobOptsSvc->m_mcMode)
    {
        saveFile->Close();

        delete vtxfit;
        return EXIT_SUCCESS;
    }

    //Like-sign
    saveFile->cd();
    TTree* saveTree_pp = new TTree("save_pp", "save_pp");
    saveTree_pp->Branch("recEvent", &recEvent, 256000, 99);

    TTree* saveTree_mm = new TTree("save_mm", "save_mm");
    saveTree_mm->Branch("recEvent", &recEvent, 256000, 99);

    for(int i = offset; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);

        if(!rawEvent->isTriggeredBy(SRawEvent::MATRIX3)) continue;
        vtxfit->setRecEvent(recEvent, 1, 1);

        if(recEvent->getNDimuons() > 0) saveTree_pp->Fill();
        recEvent->clear();
    }

    for(int i = offset; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);

        if(!rawEvent->isTriggeredBy(SRawEvent::MATRIX3)) continue;
        vtxfit->setRecEvent(recEvent, -1, -1);

        if(recEvent->getNDimuons() > 0) saveTree_mm->Fill();
        recEvent->clear();
    }
    cout << "kVertex like sign ends successfully." << endl;

    saveFile->cd();
    saveTree_pp->Write();
    saveTree_mm->Write();

    //Load track bank of mu+ and mu-
    //flag is used to indicate 1. if this event has been used, 2. the eventID
    vector<SRecTrack> ptracks[7], mtracks[7];
    vector<int> pflags[7], mflags[7];
    for(int i = 0; i < 7; ++i)
    {
        ptracks[i].reserve(25000);
        pflags[i].reserve(25000);

        mtracks[i].reserve(25000);
        mflags[i].reserve(25000);
    }

    //Extract all the tracks and put in the container
    for(int i = offset; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        if(!recEvent->isTriggeredBy(SRawEvent::MATRIX1)) continue;
        if(recEvent->getTargetPos() < 1 || rawEvent->getTargetPos() > 7) continue;

        int nTracks = recEvent->getNTracks();
        if(nTracks != 1) continue;

        SRecTrack track = recEvent->getTrack(0);
        track.setZVertex(track.getZVertex());
        if(!track.isValid()) continue;

        int index = rawEvent->getTargetPos() - 1;
        if(track.getCharge() > 0)
        {
            ptracks[index].push_back(track);
            pflags[index].push_back(rawEvent->getEventID());
        }
        else
        {
            mtracks[index].push_back(track);
            mflags[index].push_back(rawEvent->getEventID());
        }

        rawEvent->clear();
        recEvent->clear();
    }

    //Random combine, target by target
    //Unlike-sign event mixing
    saveFile->cd();
    TTree* saveTree_mix = new TTree("save_mix", "save_mix");
    saveTree_mix->Branch("recEvent", &recEvent, 256000, 99);

    dataTree->GetEntry(0);
    int runID = rawEvent->getRunID();
    TRandom rnd;
    rnd.SetSeed(runID);

    int eventID = 0;
    for(int i = 0; i < 7; ++i)
    {
        int nPlus = ptracks[i].size();
        int nMinus = mtracks[i].size();
        int nPairs = int(nPlus < nMinus ? 0.9*nPlus : 0.9*nMinus);
        cout << nPlus << " mu+ and " << nMinus << " mu- tracks with targetPos = " << i+1;
        cout << ", will generate " << nPairs << " random pairs. " << endl;

        int nTries = 0;
        int nSuccess = 0;
        while(nSuccess < nPairs && nTries - nSuccess < 10000)
        {
            ++nTries;

            int idx1 = int(rnd.Rndm()*nPlus);
            int idx2 = int(rnd.Rndm()*nMinus);

            if(pflags[i][idx1] < 0 || mflags[i][idx2] < 0) continue;
            if((ptracks[i][idx1].getTriggerRoad() > 0 && mtracks[i][idx2].getTriggerRoad() > 0) || (ptracks[i][idx1].getTriggerRoad() < 0 && mtracks[i][idx2].getTriggerRoad() < 0)) continue;
            if(fabs(ptracks[i][idx1].getVertex().Z() - mtracks[i][idx2].getVertex().Z()) > 300.) continue;

            recEvent->setEventInfo(runID, 0, eventID++);
            recEvent->setTargetPos(i+1);
            recEvent->insertTrack(ptracks[i][idx1]); pflags[i][idx1] = -pflags[i][idx1];
            recEvent->insertTrack(mtracks[i][idx2]); mflags[i][idx2] = -mflags[i][idx2];
            recEvent->setEventSource(-pflags[i][idx1], -mflags[i][idx2]);

            vtxfit->setRecEvent(recEvent);
            if(eventID % 1000 == 0) saveTree_mix->AutoSave("SaveSelf");
            ++nSuccess;

            saveTree_mix->Fill();
            recEvent->clear();
        }
        cout << "   Generated " << nSuccess << " fake pairs after " << nTries << " tries." << endl;
    }
    cout << endl;
    cout << "kVertex mixing ends successfully." << endl;

    saveFile->cd();
    saveTree_mix->Write();

    //like-sign event mixing
    saveFile->cd();
    TTree* saveTree_mix_pp = new TTree("save_mix_pp", "save_mix_pp");
    saveTree_mix_pp->Branch("recEvent", &recEvent, 256000, 99);

    TTree* saveTree_mix_mm = new TTree("save_mix_mm", "save_mix_mm");
    saveTree_mix_mm->Branch("recEvent", &recEvent, 256000, 99);

    int eventID_pp = 0;
    int eventID_mm = 0;
    for(int i = 0; i < 7; ++i)
    {
        //pp pairs
        int nPlus = ptracks[i].size();
        for(int j = 0; j < nPlus; ++j) pflags[i][j] = abs(pflags[i][j]);

        int nPairs = int(0.9*nPlus);
        cout << nPlus << " mu+ tracks with targetPos = " << i+1;
        cout << ", will generate " << nPairs << " random ++ pairs. " << endl;

        int nTries = 0;
        int nSuccess = 0;
        while(nSuccess < nPairs && nTries - nSuccess < 10000)
        {
            ++nTries;

            int idx1 = int(rnd.Rndm()*nPlus);
            int idx2 = int(rnd.Rndm()*nPlus);
            if(idx1 == idx2) continue;

            if(pflags[i][idx1] < 0 || pflags[i][idx2] < 0) continue;
            if((ptracks[i][idx1].getTriggerRoad() > 0 && ptracks[i][idx2].getTriggerRoad() > 0) || (ptracks[i][idx1].getTriggerRoad() < 0 && ptracks[i][idx2].getTriggerRoad() < 0)) continue;
            if(fabs(ptracks[i][idx1].getVertex().Z() - ptracks[i][idx2].getVertex().Z()) > 300.) continue;

            recEvent->setEventInfo(runID, 0, eventID_pp++);
            recEvent->setTargetPos(i+1);
            recEvent->insertTrack(ptracks[i][idx1]); pflags[i][idx1] = -pflags[i][idx1];
            recEvent->insertTrack(ptracks[i][idx2]); pflags[i][idx2] = -pflags[i][idx2];
            recEvent->setEventSource(-pflags[i][idx1], -pflags[i][idx2]);

            vtxfit->setRecEvent(recEvent, 1, 1);
            if(eventID_pp % 1000 == 0) saveTree_mix_pp->AutoSave("SaveSelf");
            ++nSuccess;

            saveTree_mix_pp->Fill();
            recEvent->clear();
        }
        cout << "   Generated " << nSuccess << " fake ++ pairs after " << nTries << " tries." << endl;

        //mm pairs
        int nMinus = mtracks[i].size();
        for(int j = 0; j < nMinus; ++j) mflags[i][j] = abs(mflags[i][j]);

        nPairs = int(0.9*nMinus);
        cout << nMinus << " mu- tracks with targetPos = " << i+1;
        cout << ", will generate " << nPairs << " random -- pairs. " << endl;

        nTries = 0;
        nSuccess = 0;
        while(nSuccess < nPairs && nTries - nSuccess < 10000)
        {
            ++nTries;

            int idx1 = int(rnd.Rndm()*nMinus);
            int idx2 = int(rnd.Rndm()*nMinus);
            if(idx1 == idx2) continue;

            if(mflags[i][idx1] < 0 || mflags[i][idx2] < 0) continue;
            if((mtracks[i][idx1].getTriggerRoad() > 0 && mtracks[i][idx2].getTriggerRoad() > 0) || (mtracks[i][idx1].getTriggerRoad() < 0 && mtracks[i][idx2].getTriggerRoad() < 0)) continue;
            if(fabs(mtracks[i][idx1].getVertex().Z() - mtracks[i][idx2].getVertex().Z()) > 300.) continue;

            recEvent->setEventInfo(runID, 0, eventID_mm++);
            recEvent->setTargetPos(i+1);
            recEvent->insertTrack(mtracks[i][idx1]); mflags[i][idx1] = -mflags[i][idx1];
            recEvent->insertTrack(mtracks[i][idx2]); mflags[i][idx2] = -mflags[i][idx2];
            recEvent->setEventSource(-mflags[i][idx1], -mflags[i][idx2]);

            vtxfit->setRecEvent(recEvent, -1, -1);
            if(eventID_mm % 1000 == 0) saveTree_mix_mm->AutoSave("SaveSelf");
            ++nSuccess;

            saveTree_mix_mm->Fill();
            recEvent->clear();
        }
        cout << "   Generated " << nSuccess << " fake -- pairs after " << nTries << " tries." << endl;
    }
    cout << endl;
    cout << "kVertex like-sign mixing ends successfully." << endl;

    saveFile->cd();
    saveTree_mix_pp->Write();
    saveTree_mix_mm->Write();
    saveFile->Close();

    delete vtxfit;
    return EXIT_SUCCESS;
}
