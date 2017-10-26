#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <stdio.h>

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
#include <TF1.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "SRecEvent.h"
#include "FastTracklet.h"

using namespace std;

double findCenter(TH1D* hist)
{
    if(hist->GetEffectiveEntries() < 300) return -9999.;
    if(hist->GetEffectiveEntries() < 3000)
    {
        TF1 f("func", "gaus", -5., 5.);
        hist->Fit(&f, "QNL");

        return f.GetParameter(1);
    }

    return hist->GetBinCenter(hist->GetMaximumBin());
}

void linearFit(double* par, double& a, double& b)
{
    double x[9], y[9];
    int nPars = 0;

    for(int i = 0; i < 9; ++i)
    {
        if(par[i] > -100.)
        {
            x[nPars] = i+1;
            y[nPars] = par[i];
            ++nPars;
        }
    }

    TF1 f("func", "pol1", 0, 10);
    TGraph g(nPars, x, y);
    g.Fit(&f, "QN");

    a = f.GetParameter(1);
    b = f.GetParameter(0);
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
    int propIDs[8] = {41, 42, 43, 44, 45, 46, 47, 48};
    const int nProps = 8;

    //Evaluation tree structure
    double z_exp, x_exp, y_exp, pos_exp, pos;
    int propID, elementID;

    TFile* saveFile = new TFile("prop_eval.root", "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("z_exp", &z_exp, "z_exp/D");
    saveTree->Branch("x_exp", &x_exp, "x_exp/D");
    saveTree->Branch("y_exp", &y_exp, "y_exp/D");
    saveTree->Branch("pos_exp", &pos_exp, "pos_exp/D");
    saveTree->Branch("pos", &pos, "pos/D");
    saveTree->Branch("propID", &propID, "propID/I");
    saveTree->Branch("elementID", &elementID, "elementID/I");

    //Initialization of container and hists
    //Offsets using all elements added together
    TH1D* hists[nProps/2][9];
    double offsets[nProps/2][9];

    //Other useful hodo properties
    for(int i = 0; i < nProps/2; ++i)
    {
        string detectorName = p_geomSvc->getDetectorName(propIDs[2*i]);
        double spacing = p_geomSvc->getPlaneSpacing(propIDs[2*i]);

        for(int j = 0; j < 9; j++)
        {
            stringstream suffix;
            suffix << (j+1);
            string histName = detectorName + "_" + suffix.str();

            hists[i][j] = new TH1D(histName.c_str(), histName.c_str(), 100, -spacing, spacing);
            offsets[i][j] = 9999.;
        }
    }

    //User tracks to fill residual distributions
    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);

        //Extract the tracklets, only use the first one for one less loop
        if(tracklets->GetEntries() < 1) continue;
        Tracklet* track = (Tracklet*)tracklets->At(0);

        if(track->getNHits() < 16) continue;
        if(track->getChisq() > 10.) continue;

        //Loop over prop. tube hits on each plane
        rawEvent->reIndex("oa");
        vector<Hit> hitAll = rawEvent->getAllHits();
        for(int j = 0; j < nProps; ++j)
        {
            propID = propIDs[j];

            //Expected hit position on one plane
            z_exp = p_geomSvc->getPlanePosition(propID);
            x_exp = track->getExpPositionX(z_exp);
            y_exp = track->getExpPositionY(z_exp);
            pos_exp = p_geomSvc->getUinStereoPlane(propID, x_exp, y_exp);

            //if the hit is outside of that hodo, skip
            if(!p_geomSvc->isInPlane(propID, x_exp, y_exp)) continue;

            //Get the hits on that plane, in the case of 2 hits if they are not next to each other, skip
            list<int> hitlist = rawEvent->getHitsIndexInDetector(propIDs[j]);
            elementID = -1;
            double dist_min = 1E6;
            for(std::list<int>::iterator iter = hitlist.begin(); iter != hitlist.end(); ++iter)
            {
                double dist_l = p_geomSvc->getMeasurement(propIDs[j], hitAll[*iter].elementID) + hitAll[*iter].driftDistance - pos_exp;
                double dist_r = p_geomSvc->getMeasurement(propIDs[j], hitAll[*iter].elementID) - hitAll[*iter].driftDistance - pos_exp;
                double dist = fabs(dist_l) < fabs(dist_r) ? fabs(dist_l) : fabs(dist_r);
                if(dist < dist_min)
                {
                    dist_min = dist;
                    pos = fabs(dist_l) < fabs(dist_r) ? dist_l + pos_exp : dist_r + pos_exp;
                    elementID = hitAll[*iter].elementID;
                }
            }

            if(elementID > 0)
            {
                saveTree->Fill();

                int histID = (propID - 41)/2;
                int moduleID = 8 - int(elementID - 1)/8;
                if(fabs(pos_exp - pos) < 5.08) hists[histID][moduleID]->Fill(pos_exp - pos);
            }
        }

        rawEvent->clear();
        tracklets->Clear();
    }

    //Extract center values
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 9; ++j)
        {
            offsets[i][j] = findCenter(hists[i][j]);
        }
        cout << endl;
    }

    double offsets_corr[4][9];
    for(int i = 0; i < 4; ++i)
    {
        double a, b;
        linearFit(offsets[i], a, b);
        for(int j = 0; j < 9; ++j)
        {
            if(offsets[i][j] > -100.)
            {
                offsets_corr[i][j] = offsets[i][j] + p_geomSvc->getPlaneWOffset(propIDs[2*i], j);
            }
            else
            {
                offsets_corr[i][j] = a*(j+1) + b + p_geomSvc->getPlaneWOffset(propIDs[2*i], j);
            }

            cout << i << "  " << j << "  " << hists[i][j]->GetTitle() << "  " << offsets[i][j] << "  " << offsets_corr[i][j] << "  " << p_geomSvc->getPlaneWOffset(propIDs[2*i], j) << endl;
        }
        cout << endl;
    }

    ofstream fout(argv[2], ios::out);
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 9; ++j)
        {
            fout << offsets_corr[i][j] << endl;
        }
    }

    saveFile->cd();
    saveTree->Write();
    for(int i = 0; i < 4; ++i)
    {
        for(int j = 0; j < 9; ++j)
        {
            hists[i][j]->Write();
        }
    }
    saveFile->Close();


    /*
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
    */

    return EXIT_SUCCESS;
}
