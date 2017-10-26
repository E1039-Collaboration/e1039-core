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

using namespace std;

double findCenter(TH1D *hist, double spacing)
{
    //Basic quality cut
    if(hist->GetEntries() < 150) return 9999.;
    if(hist->GetRMS() < spacing/5. && hist->GetEntries() < 1000) return 9999.;

    int nBin = hist->GetNbinsX();
    int nBinInSize = nBin/2;
    double binWidth = hist->GetBinWidth(1);

    //Search from left side
    int nEvt_max = 0;
    int index_max_left = 0;
    for(int i = 1; i <= nBinInSize; ++i)
    {
        int nEvt_curr = int(hist->Integral(i, i + nBinInSize));
        //cout << i << " : " << hist->GetBinCenter(i) << " <===> " << hist->GetBinCenter(i + nBinInSize) << " : " << nEvt_curr << " === " << nEvt_max << endl;

        if(nEvt_curr > nEvt_max)
        {
            nEvt_max = nEvt_curr;
            index_max_left = i;
        }
    }

    //Search from right side
    nEvt_max = 0;
    int index_max_right = nBin;
    for(int i = nBin; i >= nBinInSize; --i)
    {
        int nEvt_curr = int(hist->Integral(i - nBinInSize, i));

        if(nEvt_curr > nEvt_max)
        {
            nEvt_max = nEvt_curr;
            index_max_right = i;
        }
    }

    //Left-right difference should not be too large
    double left_center = hist->GetBinCenter(index_max_left + nBinInSize/2);
    double right_center = hist->GetBinCenter(index_max_right - nBinInSize/2);


    if(fabs(left_center - right_center) > 5.*binWidth) return 9999.;
    return (left_center + right_center)/2.;
}

int main(int argc, char *argv[])
{
    //Initialization of geometry and tracked data
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    SRawEvent* rawEvent = new SRawEvent();
    TClonesArray* tracklets = new TClonesArray("Tracklet");

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("tracklets", &tracklets);

    //Hodoscope IDs
    boost::array<int, 16> hodoIDs = {25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    const int nHodos = 16;

    //Evaluation tree structure
    double z_exp, x_exp, y_exp, pos_exp, pos;
    int hodoID, elementID;
    int nFired;

    TFile* saveFile = new TFile("hodo_eval.root", "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("z_exp", &z_exp, "z_exp/D");
    saveTree->Branch("x_exp", &x_exp, "x_exp/D");
    saveTree->Branch("y_exp", &y_exp, "y_exp/D");
    saveTree->Branch("pos_exp", &pos_exp, "pos_exp/D");
    saveTree->Branch("pos", &pos, "pos/D");
    saveTree->Branch("hodoID", &hodoID, "hodoID/I");
    saveTree->Branch("elementID", &elementID, "elementID/I");
    saveTree->Branch("nFired", &nFired, "nFired/I");

    //Initialization of container and hists
    //Offsets using all elements added together
    boost::array<TH1D*, nHodos> hist_all;
    boost::array<double, nHodos> offset_all;

    //Offsets using each individual elements
    boost::array<double, nHodos> offset_plane; //Global shifts extracted by fitting individual elements
    boost::array<int, nHodos> nValidEntries;   //Number of effective hits on each element
    boost::array<vector<TH1D*>, nHodos> hist;  //Residual hist
    boost::array<vector<double>, nHodos> offset; //Offset of each individual element

    //Other useful hodo properties
    boost::array<int, nHodos> nElement;
    boost::array<double, nHodos> spacing;
    for(int i = 0; i < nHodos; ++i)
    {
        nElement[i] = p_geomSvc->getPlaneNElements(hodoIDs[i]);
        spacing[i] = p_geomSvc->getCellWidth(hodoIDs[i]);
        string detectorName = p_geomSvc->getDetectorName(hodoIDs[i]);

        hist_all[i] = new TH1D(detectorName.c_str(), detectorName.c_str(), 200, -spacing[i], spacing[i]);
        offset_all[i] = 9999.;
        for(int j = 0; j < nElement[i]; j++)
        {
            stringstream suffix;
            suffix << j;
            string histName = detectorName + "_" + suffix.str();

            TH1D* hist_temp = new TH1D(histName.c_str(), histName.c_str(), 200, -spacing[i], spacing[i]);
            hist[i].push_back(hist_temp);
            offset[i].push_back(9999.);
        }
    }

    //User tracks to fill residual distributions
    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        if(tracklets->GetEntries() < 1) continue;

        rawEvent->reIndex("oa");
        vector<Hit> hitAll = rawEvent->getAllHits();

        //Only the first track is used, for simplicity
        Tracklet* _track = (Tracklet*)tracklets->At(0);
        for(int j = 0; j < nHodos; ++j)
        {
            if((rawEvent->getNHitsInDetector(hodoIDs[j]) != 1) && (rawEvent->getNHitsInDetector(hodoIDs[j]) != 2)) continue;

            //Expected hit position on one hodo plane
            z_exp = p_geomSvc->getPlanePosition(hodoIDs[j]);
            x_exp = _track->getExpPositionX(z_exp);
            y_exp = _track->getExpPositionY(z_exp);
            pos_exp = p_geomSvc->getUinStereoPlane(hodoIDs[j], x_exp, y_exp);

            //if the hit is outside of that hodo, skip
            if(!p_geomSvc->isInPlane(hodoIDs[j], x_exp, y_exp)) continue;

            //Get the hits on that plane, in the case of 2 hits if they are not next to each other, skip
            list<int> hitlist = rawEvent->getHitsIndexInDetector(hodoIDs[j]);
            nFired = hitlist.size();
            if(nFired == 2 && abs(hitAll[hitlist.front()].elementID - hitAll[hitlist.back()].elementID) > 1) continue;

            //Fill the first or the only hit
            hodoID = hitAll[hitlist.front()].detectorID;
            elementID = hitAll[hitlist.front()].elementID;
            pos = p_geomSvc->getMeasurement(hodoIDs[j], elementID);
            hist[j][elementID-1]->Fill(pos_exp - pos);
            hist_all[j]->Fill(pos_exp - pos);
            saveTree->Fill();

            //Fill the second hit if available
            if(nFired != 2) continue;
            hodoID = hitAll[hitlist.back()].detectorID;
            elementID = hitAll[hitlist.back()].elementID;
            pos = p_geomSvc->getMeasurement(hodoIDs[j], elementID);
            hist[j][elementID-1]->Fill(pos_exp - pos);
            hist_all[j]->Fill(pos_exp - pos);
            saveTree->Fill();
        }

        rawEvent->clear();
        tracklets->Clear();
    }

    //Process the residual distributions
    for(int i = 0; i < nHodos; ++i)
    {
        offset_all[i] = findCenter(hist_all[i], spacing[i]);

        offset_plane[i] = 0.;
        nValidEntries[i] = 0;
        for(int j = 0; j < nElement[i]; j++)
        {
            int nEntries = int(hist[i][j]->GetEntries());
            if(nEntries < 500)
            {
                hist[i][j]->Rebin(2);
            }

            offset[i][j] = findCenter(hist[i][j], spacing[i]);
            if(offset[i][j] < 100.)
            {
                offset_plane[i] += offset[i][j]*hist[i][j]->GetEntries();
                nValidEntries[i] += int(hist[i][j]->GetEntries());
            }
        }

        offset_plane[i] = offset_plane[i]/nValidEntries[i];
    }

    //Output alignment parameters into the txt file
    ofstream fout(argv[2], ios::out);
    for(int i = 0; i < nHodos; i++)
    {
        cout << " === " << p_geomSvc->getDetectorName(hodoIDs[i]) << "  " << offset_plane[i] << "  " << offset_all[i] << endl;

        double offset_final;
        if(fabs(offset_all[i]) < 5.)
        {
            offset_final = offset_all[i];
        }
        else if(fabs(offset_plane[i]) < 5.)
        {
            offset_final = offset_plane[i];
        }
        else
        {
            offset_final = 0.;
        }

        fout << offset_final + p_geomSvc->getPlaneWOffset(hodoIDs[i]) << endl;
        for(int j = 0; j < nElement[i]; j++)
        {
            if(offset[i][j] > 100.) offset[i][j] = offset_plane[i];
            //cout << p_geomSvc->getDetectorName(hodoIDs[i]) << "  " << j << "  " << hist[i][j]->GetEntries() << "  " << offset[i][j] << endl;
        }
    }

    //Output results to eps file
    TCanvas* c1 = new TCanvas();
    c1->Divide(4, 4);
    for(int i = 0; i < nHodos; ++i)
    {
        c1->cd(i+1);
        hist_all[i]->Draw();

        double y_max = hist_all[i]->GetBinContent(hist_all[i]->GetMaximumBin());
        TArrow* ar_center = new TArrow(offset_all[i], y_max, offset_all[i], 0., 0.01, ">");
        TArrow* ar_left = new TArrow(offset_all[i] - 0.5*spacing[i], y_max, offset_all[i] - 0.5*spacing[i], 0., 0.01, ">");
        TArrow* ar_right = new TArrow(offset_all[i] + 0.5*spacing[i], y_max, offset_all[i] + 0.5*spacing[i], 0., 0.01, ">");

        ar_center->SetLineWidth(2);
        ar_left->SetLineWidth(2);
        ar_right->SetLineWidth(2);

        ar_center->SetLineColor(kRed);
        ar_right->SetLineColor(kBlue);
        ar_left->SetLineColor(kBlue);

        ar_center->Draw();
        ar_left->Draw();
        ar_right->Draw();

        char content[100];
        sprintf(content, "Offsets: %f cm", offset_all[i]);
        TText* text = new TText();
        text->DrawTextNDC(0.1, 0.92, content);
    }

    //Save the temporary results into the ROOT file
    saveFile->cd();
    saveTree->Write();
    c1->Write();
    for(int i = 0; i < nHodos; i++)
    {
        hist_all[i]->Write();
        for(int j = 0; j < nElement[i]; j++)
        {
            hist[i][j]->Write();
            //cout << p_geomSvc->getDetectorName(hodoIDs[i]) << "  " << j << "  " << hist[i][j]->GetEntries() << "  " << findCenter(hist[i][j]) << endl;
        }
    }
    saveFile->Close();

    return EXIT_SUCCESS;
}
