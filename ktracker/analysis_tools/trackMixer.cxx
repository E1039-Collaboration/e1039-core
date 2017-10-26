#include <iostream>
#include <fstream>
#include <sstream>
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
#include <TChain.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char *argv[])
{
    //Initialize job options
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();
    jobOptsSvc->init(argv[1]);

    //Retrieve the raw event
    LogInfo("Retrieving the event stored in ROOT file ... ");
    SRawEvent* rawEvent = new SRawEvent();
    SRecEvent* recEvent = new SRecEvent();

    TFile* dataFile = new TFile(jobOptsSvc->m_inputFile.c_str(), "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("recEvent", &recEvent);

    SRecEvent* mixEvent = new SRecEvent();

    TFile* saveFile = new TFile(jobOptsSvc->m_outputFile.c_str(), "recreate");

    TTree* saveTree_mix = new TTree("save_mix", "save_mix");
    saveTree_mix->Branch("recEvent", &mixEvent, 256000, 99);

    TTree* saveTree_pp = new TTree("save_pp", "save_pp");
    saveTree_pp->Branch("recEvent", &recEvent, 256000, 99);

    TTree* saveTree_mm = new TTree("save_mm", "save_mm");
    saveTree_mm->Branch("recEvent", &recEvent, 256000, 99);

    //Initialize vertex finder
    VertexFit* vtxfit = new VertexFit();
    vtxfit->enableOptimization();

    //Load track bank of mu+ and mu-
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
    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; i++)
    {
        dataTree->GetEntry(i);
        if(!rawEvent->isTriggeredBy(SRawEvent::MATRIX1)) continue;
        if(rawEvent->getTargetPos() < 1 || rawEvent->getTargetPos() > 7) continue;

        int nTracks = recEvent->getNTracks();
        if(nTracks != 1) continue;

        SRecTrack track = recEvent->getTrack(0);
        track.setZVertex(track.getZVertex());
        if(!track.isValid()) continue;

        int index = rawEvent->getTargetPos() - 1;
        if(track.getCharge() > 0)
        {
            ptracks[index].push_back(track);
            pflags[index].push_back(1);
        }
        else
        {
            mtracks[index].push_back(track);
            mflags[index].push_back(1);
        }

        rawEvent->clear();
        recEvent->clear();
    }

    //Like-sign muon pairs
    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        vtxfit->setRecEvent(recEvent, 1, 1);

        if(recEvent->getNDimuons() > 0) saveTree_pp->Fill();
        recEvent->clear();
    }

    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        vtxfit->setRecEvent(recEvent, -1, -1);

        if(recEvent->getNDimuons() > 0) saveTree_mm->Fill();
        recEvent->clear();
    }

    saveFile->cd();
    saveTree_pp->Write();
    saveTree_mm->Write();

    //Random combination
    dataTree->GetEntry(0);
    int runID = rawEvent->getRunID();
    TRandom rnd;
    rnd.SetSeed(runID);

    int eventID = 0;
    for(int i = 0; i < 7; ++i)
    {
        int nPlus = ptracks[i].size();
        int nMinus = mtracks[i].size();
        int nPairs = int(nPlus < nMinus ? 0.8*nPlus : 0.8*nMinus);
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

            mixEvent->setEventInfo(runID, 0, eventID++);
            mixEvent->setTargetPos(i+1);
            mixEvent->insertTrack(ptracks[i][idx1]);
            pflags[i][idx1] = -1;
            mixEvent->insertTrack(mtracks[i][idx2]);
            mflags[i][idx2] = -1;

            vtxfit->setRecEvent(mixEvent);
            if(eventID % 1000 == 0) saveTree_mix->AutoSave("SaveSelf");

            ++nSuccess;

            saveTree_mix->Fill();
            mixEvent->clear();
        }
        cout << "   Generated " << nSuccess << " fake pairs after " << nTries << " tries." << endl;
    }
    cout << endl;
    cout << "kVertex_mix ends successfully." << endl;

    saveFile->cd();
    saveTree_mix->Write();
    saveFile->Close();

    delete vtxfit;

    return EXIT_SUCCESS;
}
