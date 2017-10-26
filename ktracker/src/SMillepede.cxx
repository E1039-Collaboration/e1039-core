/*
SMillepede.cxx

Implemenetation of the OOP wrapper of fortran code millepede, and also the operation
of the whole alignment process

Author: Kun Liu, liuk@fnal.gov
Created: 05-01-2013
*/

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <list>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TMath.h>
#include <TAxis.h>
#include <TF1.h>

#include "KalmanUtil.h"
#include "SMillepede.h"

#define ENABLE_REFIT

SMillepede::SMillepede()
{
    evalFile = NULL;
    evalTree = NULL;
    p_geomSvc = GeomSvc::instance();

    tracker = new KalmanFastTracking(false);
}

SMillepede::~SMillepede()
{
    if(evalFile != NULL)
    {
        for(int i = 0; i < MILLEPEDE::NPLAN; i++) delete evalHist[i];

        evalFile->cd();
        evalTree->Write();
        evalFile->Close();
    }

    delete evalNode;
    delete tracker;
}

void SMillepede::init(std::string configFileName)
{
    //Initialization of counters
    nTracks = 0;
    for(int i = 0; i < MILLEPEDE::NPLAN; i++)
    {
        nHits[i] = 0;
    }

    //Initialization of the global parameters
    for(int i = 0; i < MILLEPEDE::NGLB; i++)
    {
        par_align[i] = 0.;
        err_align[i] = 0.;
    }

    //Initialization of millepede
    initMillepede(configFileName);
}

bool SMillepede::acceptTrack(SRecTrack& recTrack)
{
    if(recTrack.getNHits() < MILLEPEDE::NHITSMIN) return false;
    if(recTrack.getChisq() > MILLEPEDE::CHISQMAX) return false;
    if(recTrack.getMomentumSt1() < MILLEPEDE::MOMMIN) return false;

    return true;
}

bool SMillepede::acceptTrack(Tracklet& track)
{
    if(track.getNHits() < MILLEPEDE::NHITSMIN) return false;
    //if(track.chisq > MILLEPEDE::CHISQMAX) return false;
    if(track.getProb() < MILLEPEDE::PROBMIN) return false;
    if(1./track.invP < MILLEPEDE::MOMMIN) return false;
    if(fabs(track.getExpPositionX(275.)) > 10.) return false;
    if(fabs(track.getExpPositionY(40.)) > 10.) return false;

    return true;
}

void SMillepede::setEvent(SRawEvent* rawEvt, SRecEvent* recEvt)
{
    rawEvent = rawEvt;
    recEvent = recEvt;

    int nTracksTotal = recEvent->getNTracks();
    for(int i = 0; i < nTracksTotal; i++)
    {
        SRecTrack trk = recEvent->getTrack(i);
        if(!acceptTrack(trk)) continue;

        addTrack(trk);
        ++nTracks;
    }
}

void SMillepede::setEvent(TClonesArray* trks)
{
    int nTracksTotal = trks->GetEntries();
    for(int i = 0; i < nTracksTotal; i++)
    {
        Tracklet* trk = (Tracklet*)trks->At(i);
        if(!acceptTrack(*trk)) continue;

        addTrack(*trk);
        ++nTracks;
    }
}

void SMillepede::addTrack(Tracklet& trk)
{
    //Push meanningful nodes
    std::vector<int> detectorIDs;
    detectorIDs.clear();
    nodes.clear();
    for(std::list<SignedHit>::iterator iter = trk.hits.begin(); iter != trk.hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;

#ifdef ENABLE_REFIT
        //Temporarily disable this hit, re-fit the track, turn on the hit, and calc chisq
        iter->hit.index = -iter->hit.index;
        tracker->fitTracklet(trk);
        iter->hit.index = -iter->hit.index;
        trk.calcChisq();
#endif

        MPNode node_real(*iter, trk);
        nodes.push_back(node_real);
        detectorIDs.push_back(iter->hit.detectorID);

        ++nHits[iter->hit.detectorID - 1];
    }

    //Push dummy nodes
    std::vector<int> detectorIDs_all;
    for(int i = 1; i < MILLEPEDE::NPLAN; i++) detectorIDs_all.push_back(i);

    std::vector<int> detectorIDs_miss(nChamberPlanes);
    std::vector<int>::iterator iter = std::set_difference(detectorIDs_all.begin(), detectorIDs_all.end(), detectorIDs.begin(), detectorIDs.end(), detectorIDs_miss.begin());
    detectorIDs_miss.resize(iter - detectorIDs_miss.begin());
    for(unsigned int i = 0; i < detectorIDs_miss.size(); i++)
    {
        MPNode node_mp(detectorIDs_miss[i]);
        nodes.push_back(node_mp);
    }
    std::sort(nodes.begin(), nodes.end());

    //Add vertex point constrain
    //addVertex();

    setSingleTrack();
    fillEvaluationTree();
}

void SMillepede::addTrack(SRecTrack& trk)
{
    std::vector<int> detectorIDs_all;
    for(int i = 1; i < MILLEPEDE::NPLAN; i++) detectorIDs_all.push_back(i);

    //Prepare the node list with real hits
    std::vector<int> detectorIDs;
    detectorIDs.clear();
    nodes.clear();
    for(int i = 0; i < trk.getNHits(); i++)
    {
        int hitIndex = trk.getHitIndex(i);
        Hit h = rawEvent->getHit(recEvent->getLocalID(abs(hitIndex)));
        h.driftDistance = h.driftDistance*(hitIndex/abs(hitIndex));

        Node node_kalman(h);
        node_kalman.setZ(trk.getZ(i));
        node_kalman.getSmoothed()._state_kf = trk.getStateVector(i);
        node_kalman.getSmoothed()._covar_kf = trk.getCovariance(i);

        MPNode node_mp(node_kalman);
        nodes.push_back(node_mp);

        detectorIDs.push_back(h.detectorID);
        ++nHits[h.detectorID - 1];
    }
    std::sort(detectorIDs.begin(), detectorIDs.end());

    //Insert dummy MPNodes for the detectors without hits
    std::vector<int> detectorIDs_miss(nChamberPlanes);
    std::vector<int>::iterator iter = std::set_difference(detectorIDs_all.begin(), detectorIDs_all.end(), detectorIDs.begin(), detectorIDs.end(), detectorIDs_miss.begin());
    detectorIDs_miss.resize(iter - detectorIDs_miss.begin());
    for(unsigned int i = 0; i < detectorIDs_miss.size(); i++)
    {
        MPNode node_mp(detectorIDs_miss[i]);
        nodes.push_back(node_mp);
    }
    std::sort(nodes.begin(), nodes.end());

    //Add vertex point constrain
    //addVertex();

    setSingleTrack();
    fillEvaluationTree();
}

void SMillepede::addVertex()
{
    //Find the most upstream valid node info
    double tx = 0.;
    double ty = 0.;
    double x0 = 0.;
    double y0 = 0.;
    for(std::vector<MPNode>::iterator node = nodes.begin(); node != nodes.end(); ++node)
    {
        if(node->isValid())
        {
            tx = node->tx;
            ty = node->ty;
            x0 = node->x0;
            y0 = node->y0;

            break;
        }
    }

    //Vertex constrain for x0
    MPNode node_x0(0);
    node_x0.flag = true;

    node_x0.meas = 0.;
    node_x0.sigma = 5.;

    node_x0.tx = tx;
    node_x0.ty = ty;
    node_x0.x0 = x0;
    node_x0.y0 = y0;
    node_x0.z = 275. - Z_REF;

    node_x0.setDerivatives(node_x0.z, 1., 0., tx, ty, x0, y0);
    nodes.push_back(node_x0);

    //Vertex constrain for x0
    MPNode node_y0(-1);
    node_y0.flag = true;

    node_y0.meas = 0.;
    node_y0.sigma = 5.;

    node_y0.tx = tx;
    node_y0.ty = ty;
    node_y0.x0 = x0;
    node_y0.y0 = y0;
    node_y0.z = 40. - Z_REF;

    node_y0.setDerivatives(node_y0.z, 0., 1., tx, ty, x0, y0);
    nodes.push_back(node_y0);

    std::sort(nodes.begin(), nodes.end());
}

void SMillepede::constrainDetectors(int detectorID1, int detectorID2, int paraID)
{
    int index1 = (detectorID1-1)*MILLEPEDE::NPARPLAN + paraID;
    int index2 = (detectorID2-1)*MILLEPEDE::NPARPLAN + paraID;

    float rhs = 0.;
    float dercs[MILLEPEDE::NGLB];
    for(int i = 0; i < MILLEPEDE::NGLB; i++) dercs[i] = 0;

    dercs[index1] = 1.;
    dercs[index2] = -1.;

    constf_(dercs, &rhs);
}

void SMillepede::initMillepede(std::string configFileName)
{
    //Define the dimension of arrays in millepede
    int nGlobal = MILLEPEDE::NGLB;
    int nLocal = MILLEPEDE::NPARTRK;
    int nStdev = MILLEPEDE::NSTDEV;
    int iPrint = 1;

    initgl_(&nGlobal, &nLocal, &nStdev, &iPrint);

    //Set parameter initial value and resolution parameter
    double err_z[MILLEPEDE::NPLAN], err_phi[MILLEPEDE::NPLAN], err_w[MILLEPEDE::NPLAN];
    for(int i = 1; i <= MILLEPEDE::NPLAN; ++i)
    {
        err_z[i-1] = 0.1;
        err_phi[i-1] = 0.005;
        err_w[i-1] = 0.05;
    }

    std::fstream conf;
    conf.open(configFileName.c_str(), std::ios::in);
    if(conf)
    {
        int detectorID;
        double err1, err2, err3;
        char buf[200];
        while(conf.getline(buf, 200))
        {
            std::istringstream stringBuf(buf);
            stringBuf >> detectorID >> err1 >> err2 >> err3;

            err_z[detectorID-1] = err1;
            err_phi[detectorID-1] = err2;
            err_w[detectorID-1] = err3;
        }
    }
    conf.close();

    for(int i = 1; i <= MILLEPEDE::NPLAN; i++)
    {
        setDetectorParError(i, 0, err_z[i-1]);
        setDetectorParError(i, 1, err_phi[i-1]);
        setDetectorParError(i, 2, err_w[i-1]);
    }

    //fix one detector as-is, it needs to be done here as it sets the center value as well
    fixDetectorParameter(p_geomSvc->getDetectorID("D2X"), 0, 0.);
    fixDetectorParameter(p_geomSvc->getDetectorID("D2X"), 1, 0.);
    fixDetectorParameter(p_geomSvc->getDetectorID("D2X"), 2, 0.);

    // Now pass the info above to millepede
    parglo_(par_align);
    for(int i = 0; i < MILLEPEDE::NGLB; i++)
    {
        int index = i+1;
        parsig_(&index, &err_align[i]);
    }

    //Constrains
    //1. Fix adjacent planes to be same
    for(int i = 1; i <= MILLEPEDE::NPLAN; i += 2)
    {
        constrainDetectors(i, i+1, 0);
        constrainDetectors(i, i+1, 1);
        //constrainDetectors(i, i+1, 2);

        //for station 1 or 3+/-, w offset is also fixed together
        if((i > 6 && i < 13) || i > 18) constrainDetectors(i, i+1, 2);
    }

    //2. Fix all 6 planes of station 1/3+/3- to be the same in z and phi
    for(int i = 7; i < 10; i += 2)
    {
        constrainDetectors(i, i+1, 0);
        constrainDetectors(i, i+1, 1);
    }

    for(int i = 20; i < 23; i += 2)
    {
        constrainDetectors(i, i+1, 0);
        constrainDetectors(i, i+1, 1);
    }

    for(int i = 26; i < 29; i += 2)
    {
        constrainDetectors(i, i+1, 0);
        constrainDetectors(i, i+1, 1);
    }

    //3. the global offsets of station 1, 3+ and 3- is fixed
    float rhs = 0.;
    float dercs[MILLEPEDE::NGLB];

    //1
    for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    dercs[MILLEPEDE::NPARPLAN*6 + 2] = 1.;
    dercs[MILLEPEDE::NPARPLAN*8 + 2] = -(p_geomSvc->getCostheta(7) + p_geomSvc->getCostheta(11));
    dercs[MILLEPEDE::NPARPLAN*10 + 2] = 1.;

    constf_(dercs, &rhs);

    //3+
    for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    dercs[MILLEPEDE::NPARPLAN*18 + 2] = 1.;
    dercs[MILLEPEDE::NPARPLAN*20 + 2] = -(p_geomSvc->getCostheta(19) + p_geomSvc->getCostheta(23));
    dercs[MILLEPEDE::NPARPLAN*22 + 2] = 1.;

    constf_(dercs, &rhs);

    //3-
    for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    dercs[MILLEPEDE::NPARPLAN*24 + 2] = 1.;
    dercs[MILLEPEDE::NPARPLAN*26 + 2] = -(p_geomSvc->getCostheta(25) + p_geomSvc->getCostheta(29));
    dercs[MILLEPEDE::NPARPLAN*28 + 2] = 1.;

    constf_(dercs, &rhs);

    //Fix global positions
    /*
    for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    for(int i = 0; i < MILLEPEDE::NPLAN; i++) dercs[i*MILLEPEDE::NPARPLAN + 0] = 1.;
    constf_(dercs, &rhs);

    for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    for(int i = 0; i < MILLEPEDE::NPLAN; i++) dercs[i*MILLEPEDE::NPARPLAN + 1] = 1.;
    constf_(dercs, &rhs);

    //for(int k = 0; k < MILLEPEDE::NGLB; k++) dercs[k] = 0.;
    //for(int i = 0; i < MILLEPEDE::NPLAN; i++) dercs[i*MILLEPEDE::NPARPLAN + 2] = 1.;
    //constf_(dercs, &rhs);
    */

    //Initialize the iteration setting
    int iUnit = 11;
    float cFactor = 1000.;
    initun_(&iUnit, &cFactor);
}

void SMillepede::fixDetectorParameter(int detectorID, int parameterID, float val)
{
    //initial error
    par_align[MILLEPEDE::NPARPLAN*(detectorID - 1) + parameterID] = val;
    err_align[MILLEPEDE::NPARPLAN*(detectorID - 1) + parameterID] = 0.;

    //Contrain
    float dercs[MILLEPEDE::NGLB];
    float rhs = val;

    for(int i = 0; i < MILLEPEDE::NGLB; i++) dercs[i] = 0.;
    dercs[MILLEPEDE::NPARPLAN*(detectorID - 1) + parameterID] = 1.;

    constf_(dercs, &rhs);
}

void SMillepede::setSingleTrack()
{
    using namespace MILLEPEDE;

    //Global and local derivatives
    float dergb[NGLB];
    float derlc[NPARTRK];
    float meas;
    float sigma;

    //Fill the nodes to derivative arrays
    for(std::vector<MPNode>::iterator node = nodes.begin(); node != nodes.end(); ++node)
    {
        if(!node->isValid()) continue;

        //Initialization of all parameters
        zerloc_(dergb, derlc);

        //Get measurements
        int index = node->detectorID - 1;
        meas = node->meas;
        sigma = node->sigma;

        //Fill local derivarives
        derlc[0] = node->dwdx;
        derlc[1] = node->dwdy;
        derlc[2] = node->dwdtx;
        derlc[3] = node->dwdty;

        //Fill global derivatives
        if(index >= 0)
        {
            dergb[NPARPLAN*index + 0] = node->dwdz;
            dergb[NPARPLAN*index + 1] = node->dwdphi;
            dergb[NPARPLAN*index + 2] = node->dwdw;
        }

        //Book the local/global derivatives, measurement and error
        equloc_(dergb, derlc, &meas, &sigma);
    }

    //Perform local track fit
    fitloc_();
}

void SMillepede::fitAlignment()
{
    //Perform global fits
    fitglo_(par_align);

    //Retrieve error of the aligned parameters
    for(int i = 0; i < MILLEPEDE::NGLB; i++)
    {
        err_align[i] = errpar_(&i);
    }
}

void SMillepede::printResults(std::string outputFileName, std::string increamentFileName)
{
    using namespace MILLEPEDE;
    //using namespace std;
    std::cout << nTracks << " tracks are used in the alignment. " << std::endl;

    fstream fout1, fout2;
    fout1.open(outputFileName.c_str(), std::ios::out);
    fout2.open(increamentFileName.c_str(), std::ios::out);
    for(int i = 0; i < NPLAN; i++)
    {
        std::cout << i+1 << "   " << p_geomSvc->getDetectorName(i+1) << "     " << nHits[i]
                  << "     " << par_align[i*NPARPLAN + 0] << " +/- " << err_align[i*NPARPLAN + 0]
                  << "     " << par_align[i*NPARPLAN + 1] << " +/- " << err_align[i*NPARPLAN + 1]
                  << "     " << par_align[i*NPARPLAN + 2] << " +/- " << err_align[i*NPARPLAN + 2]
                  << "     " << evalHist[i]->GetMean() << " +/- " << evalHist[i]->GetRMS() << std::endl;

        fout1 << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 0] + p_geomSvc->getPlaneZOffset(i+1)
              << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 1] + p_geomSvc->getPlanePhiOffset(i+1)
              << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 2] + p_geomSvc->getPlaneWOffset(i+1)
              << std::setw(20) << std::setiosflags(std::ios::right) << evalHist[i]->GetRMS() << std::endl;

        fout2 << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 0]
              << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 1]
              << std::setw(20) << std::setiosflags(std::ios::right) << par_align[i*NPARPLAN + 2]
              << std::setw(20) << std::setiosflags(std::ios::right) << evalHist[i]->GetMean()
              << std::setw(20) << std::setiosflags(std::ios::right) << evalHist[i]->GetRMS() << std::endl;
    }

    fout1.close();
    fout2.close();
}

void SMillepede::printQAPlots()
{
    using namespace MILLEPEDE;

    double dID[MILLEPEDE::NPLAN], edID[MILLEPEDE::NPLAN];
    double dw[MILLEPEDE::NPLAN], edw[MILLEPEDE::NPLAN];
    double dphi[MILLEPEDE::NPLAN], edphi[MILLEPEDE::NPLAN];
    double dz[MILLEPEDE::NPLAN], edz[MILLEPEDE::NPLAN];
    for(int i = 0; i < MILLEPEDE::NPLAN; i++)
    {
        dID[i] = i+1;
        edID[i] = 0.;

        dz[i] = par_align[i*NPARPLAN + 0] + p_geomSvc->getPlaneZOffset(i+1);
        edz[i] = err_align[i*NPARPLAN + 0];

        dphi[i] = par_align[i*NPARPLAN + 1] + p_geomSvc->getPlanePhiOffset(i+1);
        edphi[i] = err_align[i*NPARPLAN + 1];

        dw[i] = par_align[i*NPARPLAN + 2] + p_geomSvc->getPlaneWOffset(i+1);
        edw[i] = err_align[i*NPARPLAN + 2];
    }

    TGraphErrors w_vs_id(MILLEPEDE::NPLAN, dID, dw, edID, edw);
    TGraphErrors phi_vs_id(MILLEPEDE::NPLAN, dID, dphi, edID, edphi);
    TGraphErrors z_vs_id(MILLEPEDE::NPLAN, dID, dz, edID, edz);

    w_vs_id.SetMarkerStyle(8);
    phi_vs_id.SetMarkerStyle(8);
    z_vs_id.SetMarkerStyle(8);

    w_vs_id.SetTitle("");
    phi_vs_id.SetTitle("");
    z_vs_id.SetTitle("");

    w_vs_id.GetXaxis()->SetTitle("detectorID");
    phi_vs_id.GetXaxis()->SetTitle("detectorID");
    z_vs_id.GetXaxis()->SetTitle("detectorID");

    w_vs_id.GetYaxis()->SetTitle("#Deltau (cm)");
    phi_vs_id.GetYaxis()->SetTitle("#Delta#theta (rad)");
    z_vs_id.GetYaxis()->SetTitle("#Deltaz (cm)");

    w_vs_id.GetYaxis()->CenterTitle();
    phi_vs_id.GetYaxis()->CenterTitle();
    z_vs_id.GetYaxis()->CenterTitle();

    TCanvas* c1 = new TCanvas("w_vs_id", "w_vs_id");
    c1->cd();
    c1->SetGridx();
    c1->SetGridy();
    w_vs_id.Draw("APL");
    c1->SaveAs("w_vs_id.eps");

    TCanvas* c2 = new TCanvas("phi_vs_id", "phi_vs_id");
    c2->cd();
    c2->SetGridx();
    c2->SetGridy();
    phi_vs_id.Draw("APL");
    c2->SaveAs("phi_vs_id.eps");

    TCanvas* c3 = new TCanvas("z_vs_id", "z_vs_id");
    c3->cd();
    c3->SetGridx();
    c3->SetGridy();
    z_vs_id.Draw("APL");
    c3->SaveAs("z_vs_id.eps");

    if(evalTree == NULL) return;

    TCanvas* c4 = new TCanvas("residuals", "residuals");
    c4->Divide(6, nChamberPlanes/6);
    for(int i = 0; i < MILLEPEDE::NPLAN; i++)
    {
        c4->cd(i+1);
        evalHist[i]->Draw();
    }
    c4->SaveAs("residuals.eps");

    if(evalFile != NULL)
    {
        evalFile->cd();
        c1->Write();
        c2->Write();
        c3->Write();
        c4->Write();

        for(int i = 0; i < MILLEPEDE::NPLAN; i++) evalHist[i]->Write();
    }
}

void SMillepede::bookEvaluationTree(std::string evalFileName)
{
    evalFile = new TFile(evalFileName.c_str(), "recreate");
    evalTree = new TTree("save", "save");
    evalNode = new MPNode(0);

    evalTree->Branch("runID", &runID, "runID/I");
    evalTree->Branch("eventID", &eventID, "eventID/I");
    evalTree->Branch("MPnode", &evalNode, 256000, 99);

    //Evaluation histigrams
    for(int i = 0; i < MILLEPEDE::NPLAN; i++)
    {
        evalHist[i] = new TH1D(p_geomSvc->getDetectorName(i+1).c_str(), p_geomSvc->getDetectorName(i+1).c_str(), 100, -0.5*p_geomSvc->getPlaneSpacing(i+1), 0.5*p_geomSvc->getPlaneSpacing(i+1));
    }
}

void SMillepede::fillEvaluationTree()
{
    if(evalFile == NULL) return;

    //runID = rawEvent->getRunID();
    //eventID = rawEvent->getEventID();
    for(std::vector<MPNode>::iterator node = nodes.begin(); node != nodes.end(); ++node)
    {
        if(!node->isValid()) continue;

        *evalNode = *node;
        evalTree->Fill();
        if(node->detectorID > 0) evalHist[node->detectorID-1]->Fill(node->meas);
    }
}
