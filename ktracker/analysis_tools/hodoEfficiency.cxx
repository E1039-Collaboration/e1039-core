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
#include "EventReducer.h"
#include "JobOptsSvc.h"

using namespace std;

int main(int argc, char *argv[])
{
    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();

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

    //Hodoscope IDs
    boost::array<int, 16> hodoIDs = {25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40};
    const int nHodos = 16;

    //Evaluation tree structure
    double z_exp, x_exp, y_exp, pos_exp, pos;
    double tdcTime;
    int hodoID, elementID;
    int flag; //flag = 1, should have fired, and did; flag = 0, should have fired, but not

    TFile* saveFile = new TFile(argv[2], "recreate");
    TTree* saveTree = new TTree("save", "save");

    saveTree->Branch("z_exp", &z_exp, "z_exp/D");
    saveTree->Branch("x_exp", &x_exp, "x_exp/D");
    saveTree->Branch("y_exp", &y_exp, "y_exp/D");
    saveTree->Branch("tdcTime", &tdcTime, "tdcTime/D");
    saveTree->Branch("pos_exp", &pos_exp, "pos_exp/D");
    saveTree->Branch("pos", &pos, "pos/D");
    saveTree->Branch("hodoID", &hodoID, "hodoID/I");
    saveTree->Branch("elementID", &elementID, "elementID/I");
    saveTree->Branch("flag", &flag, "flag/I");

    //User tracks to fill residual distributions
    int nEvtMax = dataTree->GetEntries();
    for(int i = 0; i < nEvtMax; ++i)
    {
        dataTree->GetEntry(i);
        if(tracklets->GetEntries() < 1) continue;
        reducer->reduceEvent(rawEvent);

        //Only the first track is used, for simplicity
        Tracklet* _track = (Tracklet*)tracklets->At(0);
        if(_track->getNHits() < 15) continue;
        //if(_track->getProb() < 0.8) continue;
        //if(fabs(_track->ty) < 0.01) continue;

        for(int j = 0; j < nHodos; ++j)
        {
            //Expected hit position on one hodo plane
            hodoID = hodoIDs[j];
            z_exp = p_geomSvc->getPlanePosition(hodoID);
            x_exp = _track->getExpPositionX(z_exp);
            y_exp = _track->getExpPositionY(z_exp);
            pos_exp = p_geomSvc->getUinStereoPlane(hodoID, x_exp, y_exp);

            if(!p_geomSvc->isInPlane(hodoID, x_exp, y_exp)) continue;

            //Get the elementID_expected
            elementID = p_geomSvc->getExpElementID(hodoID, pos_exp);
            Hit hit_exp = rawEvent->getHit(hodoID, elementID);

            if(hit_exp.detectorID > 0)
            {
                flag = 1;
                tdcTime = hit_exp.tdcTime;
                pos = hit_exp.pos;
            }
            else
            {
                flag = 2;
                tdcTime = -999.;
                pos = -999.;
            }
            if(elementID > 0 && elementID <= p_geomSvc->getPlaneNElements(hodoID)) saveTree->Fill();

            //If the expected hit position happen to be also on the other paddle
            if(fabs(pos - pos_exp) > 0.5*p_geomSvc->getPlaneSpacing(hodoID) && fabs(pos - pos_exp) < 0.5*p_geomSvc->getCellWidth(hodoID))
            {
                elementID = pos_exp < pos ? elementID - 1 : elementID + 1;
                if(elementID < 1 || elementID > p_geomSvc->getPlaneNElements(hodoID)) continue;

                Hit hit_adj = rawEvent->getHit(hodoID, elementID);
                if(hit_adj.detectorID > 0)
                {
                    flag = 1;
                    tdcTime = hit_adj.tdcTime;
                    pos = hit_adj.pos;
                }
                else
                {
                    flag = 2;
                    tdcTime = -999.;
                    pos = -999.;
                }
                saveTree->Fill();
            }
        }

        rawEvent->clear();
        tracklets->Clear();
    }

    //Save the temporary results into the ROOT file
    saveFile->cd();
    saveTree->Write();
    saveFile->Close();

    return 1;
}
