#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

#include <boost/array.hpp>

#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TRandom.h>
#include <TMatrixD.h>
#include <TLorentzVector.h>
#include <TH1D.h>
#include <TH1I.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TClonesArray.h>
#include <TArrow.h>
#include <TPaveText.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "SRecEvent.h"
#include "FastTracklet.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char *argv[])
{
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
    p_jobOptsSvc->init();

    //Initialization of geometry and tracked data
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    SRawEvent* rawEvent = new SRawEvent();
    TClonesArray* tracklets = new TClonesArray("Tracklet");

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("tracklets", &tracklets);

    int nPerfect[25];
    int nMiss[25];
    for(int i = 0; i < 25; ++i)
    {
        nPerfect[i] = 0;
        nMiss[i] = 0;
    }

    TH1D* hists[25];
    for(int i = 1; i < 25; ++i)
    {
        hists[i] = new TH1D(p_geomSvc->getDetectorName(i).c_str(), p_geomSvc->getDetectorName(i).c_str(), 20, -0.5*p_geomSvc->getPlaneScaleX(i), 0.5*p_geomSvc->getPlaneScaleX(i));
        hists[i]->GetXaxis()->SetTitle("w (cm)");
    }

    int nSumTop = 171;
    int nSumBot = 207;

    //User tracks to fill residual distributions
    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        if(tracklets->GetEntries() == 0) continue;

        for(int j = 0; j < tracklets->GetEntries(); ++j)
        {
            Tracklet* track = (Tracklet*)tracklets->At(j);
            if(track->getNHits() < 17) continue;
            if(track->getChisq() > 10) continue;

            int nSum = 0;
            for(list<SignedHit>::iterator hit = track->hits.begin(); hit != track->hits.end(); ++hit)
            {
                if(hit->hit.index < 0) continue;
                nSum += hit->hit.detectorID;

                if(track->getNHits() == 18)  ++nPerfect[hit->hit.detectorID];
            }

            int iMiss = nSum > nSumTop ? nSumBot - nSum : nSumTop - nSum;
            ++nMiss[iMiss];

            if(iMiss > 0) hists[iMiss]->Fill(track->getExpPositionW(iMiss));

            //cout << track->getNHits() << "  " << nSum << "  " << iMiss << endl;
        }

        rawEvent->clear();
        tracklets->Clear();
    }

    double p[25];
    double dp[25];
    for(int i = 1; i < 25; ++i)
    {
        cout << i << "   " << nPerfect[i] << "   " << nMiss[i] << "   ";
        if(nPerfect[i] + nMiss[i] != 0)
        {
            p[i] = double(nPerfect[i])/double(nPerfect[i] + nMiss[i]);
            dp[i] = sqrt(p[i]*(1.-p[i])/(nPerfect[i] + nMiss[i]));
            cout << p[i] << "   " << dp[i] << endl;
        }
        else
        {
            p[i] = 0.;
            dp[i] = 0.;
            cout << endl;
        }
    }

    //Now make the plots
    TH1D* hist = new TH1D("hist", "Chamber efficiency", 24, 0.5, 24.5);
    for(int i = 1; i <= 24; ++i)
    {
        hist->SetBinContent(i, p[i]);
        hist->SetBinError(i, dp[i]);
        //hist->GetXaxis()->SetBinLabel(i, detectorName[i-1].c_str());
        hist->GetXaxis()->SetBinLabel(i, p_geomSvc->getDetectorName(i).c_str());
    }

    hist->SetStats(kFALSE);
    hist->SetMarkerStyle(8);
    hist->SetMarkerColor(2);
    hist->SetLineColor(2);
    hist->SetLineWidth(2);
    hist->GetYaxis()->SetRangeUser(0.7, 1.1);
    hist->GetYaxis()->SetTitle("Efficiency");
    hist->GetYaxis()->CenterTitle();

    TCanvas* c1 = new TCanvas();
    c1->SetGrid(1, 1);
    c1->cd(); hist->Draw();
    c1->SaveAs(argv[2]);

    TCanvas* c2 = new TCanvas();
    c2->Divide(6, 4);
    for(int i = 1; i <= 24; ++i)
    {
        c2->cd(i); hists[i]->Draw();
    }
    c2->SaveAs(argv[3]);

    return 1;
}
