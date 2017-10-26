/*
SMillepede.h

OOP wrapper of the fortran program millepede, defines interface with fortran and
conversion from Kalman nodes to alignment nodes

Author: Kun Liu, liuk@fnal.gov
Created: Apr. 29, 2013
*/

#ifndef _SMILLEPEDE_H
#define _SMILLEPEDE_H

#include "GlobalConsts.h"

#include <string>

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TClonesArray.h>

#include "GeomSvc.h"
#include "SRecEvent.h"
#include "SMillepedeUtil.h"
#include "FastTracklet.h"
#include "KalmanFastTracking.h"

///Interface for cross-compilation with fortran millepede program
extern "C"
{
    //Initialize global alignment parameter arrays
    extern void initgl_(int*, int*, int*, int*);

    //Optional: define sigma for single parameter
    extern void parsig_(int*, float*);

    //Optional: unit for iterations
    extern void initun_(int*, float*);

    //Optional: set initial value of global parameters
    extern void parglo_(float*);

    //Optional: constraints
    extern void constf_(float*, float*);

    //Set all local arrays to zero
    extern void zerloc_(float*, float*);

    //Equations for local fit
    extern void equloc_(float*, float*, float*, float*);

    //Local parameter fit (+entry KILLOC)
    extern void fitloc_();

    //Global parameter fit
    extern void fitglo_(float*);

    //Retrieve error for a given parameter
    extern float errpar_(int*);
}

///Parameters for alignment pocedure
namespace MILLEPEDE
{
    //Maximum number of detector planes
    static const int NPLAN = nChamberPlanes;

    //Number of track parameters
    static const int NPARTRK = 4;

    //Number of alignment parameters per plane
    static const int NPARPLAN = 3;

    //Total number of global parameters
    static const int NGLB = NPLAN*NPARPLAN;

    //Standard deviation cut of the residule
    static const int NSTDEV = 3;

    //Minimun number of hits
    static const int NHITSMIN = 15;

    //Maximum allowed chi square/prob
    static const double CHISQMAX = 8.;
    static const double PROBMIN = 0.9;

    //Lowest allowed momentum
    static const double MOMMIN = 25.;
};

class SMillepede
{
public:
    SMillepede();
    ~SMillepede();

    //Initialization
    void init(std::string configFileName = "mille.conf");

    //Set the reconstructed event
    void setEvent(SRawEvent* rawEvt, SRecEvent* recEvt);
    void setEvent(TClonesArray* trks);

    //Fill track info.
    void addTrack(SRecTrack& trk);
    void addTrack(Tracklet& trk);
    void addVertex();

    //Simutaneous fit to global parameters
    void fitAlignment();

    //Fix detector parameters
    void fixDetectorParameter(int detectorID, int parameterID, float val = 0.);
    void setDetectorParError(int detectorID, int parameterID, float err) { err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + parameterID] = err; }
    void setDetectorParameter(int detectorID, int parameterID, float val) { par_align[(detectorID-1)*MILLEPEDE::NPARPLAN + parameterID] = val; }
    void constrainDetectors(int detectorID1, int detectorID2, int parameterID);

    //Book/Fill evaluation tree
    void bookEvaluationTree(std::string evalFileName);
    void fillEvaluationTree();

    //Output alignment results
    void printResults(std::string outputFileName, std::string incrementFileName);
    void printQAPlots();

    int getNHitsOnDetector(int detectorID) { return nHits[detectorID-1]; }
    double getZOffsetVal(int detectorID)   { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 0]; }
    double getWOffsetVal(int detectorID)   { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 2]; }
    double getPhiOffsetVal(int detectorID) { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 1]; }
    double getZOffsetErr(int detectorID)   { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 0]; }
    double getWOffsetErr(int detectorID)   { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 2]; }
    double getPhiOffsetErr(int detectorID) { return err_align[(detectorID-1)*MILLEPEDE::NPARPLAN + 1]; }

    //Wrapping with precision conversion and clearer method name
    void initMillepede(std::string configFileName);
    void setSingleTrack();

    //Track quality cut for alignment
    bool acceptTrack(SRecTrack& recTrack);
    bool acceptTrack(Tracklet& track);

private:
    //Pointer to the raw and rec events
    SRawEvent* rawEvent;
    SRecEvent* recEvent;

    //Array of the alignment parameters
    float par_align[MILLEPEDE::NGLB];
    float err_align[MILLEPEDE::NGLB];

    //vector of nodes from a single track
    std::vector<MPNode> nodes;

    //Pointer to geometry service
    GeomSvc* p_geomSvc;

    //Number of tracks and hits used
    int nTracks;
    int nHits[MILLEPEDE::NPLAN];

    //Tracker used for track re-fitting
    KalmanFastTracking* tracker;

    //Evaluation files and trees
    TFile* evalFile;
    TTree* evalTree;
    TH1D* evalHist[MILLEPEDE::NPLAN];
    int runID;
    int eventID;
    MPNode *evalNode;

    //float dergb[MILLEPEDE::NGLB];
    //float derlc[MILLEPEDE::NPARTRK];
};

#endif
