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
#include <TCanvas.h>
#include <TH1D.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"
#include "KalmanFitter.h"
#include "VertexFit.h"
#include "SRecEvent.h"

using namespace std;

int main(int argc, char *argv[])
{
    //Initialize geometry service
    LogInfo("Initializing geometry service ... ");
    GeomSvc* geometrySvc = GeomSvc::instance();
    geometrySvc->init();

    //Retrieve the raw event
    LogInfo("Retrieving the event stored in ROOT file ... ");
    SRawMCEvent* rawEvent = new SRawMCEvent();
    SRecEvent* recEvent = new SRecEvent();

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("recEvent", &recEvent);

    TH1D* hist_st3 = new TH1D("hist_st3", "hist_st3", 100, -0.001, 0.001);
    TH1D* hist_st2 = new TH1D("hist_st2", "hist_st2", 100, -0.001, 0.001);
    TH1D* hist_st1 = new TH1D("hist_st1", "hist_st1", 100, -0.001, 0.001);
    TH1D* hist_mom = new TH1D("hist_mom", "hist_mom", 100, -0.1, 0.1);

    for(int i = 0; i < dataTree->GetEntries(); i++)
    {
        dataTree->GetEntry(i);

        double px1, py1, pz1;
        double px3, py3, pz3;
        int nTracks = recEvent->getNTracks();
        for(int j = 0; j < nTracks; ++j)
        {
            SRecTrack& recTrack = recEvent->getTrack(j);

            double mom = recTrack.getMomentumSt1(px1, py1, pz1);
            recTrack.getMomentumSt3(px3, py3, pz3);

            if(recTrack.getCharge() > 0)
            {
                hist_st1->Fill(rawEvent->p_station1[0].Px()/rawEvent->p_station1[0].Pz() - px1/pz1);
                hist_st3->Fill(rawEvent->p_station3[0].Px()/rawEvent->p_station3[0].Pz() - px3/pz3);
                hist_mom->Fill((1./rawEvent->p_station1[0].Mag() - 1./mom)*rawEvent->p_station1[0].Mag());
            }
            else
            {
                hist_st1->Fill(rawEvent->p_station1[1].Px()/rawEvent->p_station1[1].Pz() - px1/pz1);
                hist_st3->Fill(rawEvent->p_station3[1].Px()/rawEvent->p_station3[1].Pz() - px3/pz3);
                hist_mom->Fill((1./rawEvent->p_station1[1].Mag() - 1./mom)*rawEvent->p_station1[1].Mag());
            }
        }

        rawEvent->clear();
        recEvent->clear();
    }

    TCanvas* c1 = new TCanvas();
    c1->Divide(2, 2);

    c1->cd(1);
    hist_st3->Draw();
    c1->cd(2);
    hist_st2->Draw();
    c1->cd(3);
    hist_st1->Draw();
    c1->cd(4);
    hist_mom->Draw();

    c1->SaveAs(argv[2]);

    return EXIT_SUCCESS;
}
