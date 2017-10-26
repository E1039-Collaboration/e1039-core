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
#include "JobOptsSvc.h"
#include "EventReducer.h"

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
    JobOptsSvc* jobOptsSvc = JobOptsSvc::instance();

    //Initialization of geometry and tracked data
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init();

    SRawEvent* rawEvent = new SRawEvent();
    TClonesArray* tracklets = new TClonesArray("Tracklet");

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree*)dataFile->Get("save");

    dataTree->SetBranchAddress("rawEvent", &rawEvent);
    dataTree->SetBranchAddress("tracklets", &tracklets);

    EventReducer* reducer = new EventReducer("ao");

    //prop tube IDs
    int propIDs[8] = {47, 48, 49, 50, 51, 52, 53, 54};
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
        reducer->reduceEvent(rawEvent);
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

                int histID = (propID - 47)/2;
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

    //fit the center values w.r.t moduleID
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

    //final output
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

    return 1;
}
