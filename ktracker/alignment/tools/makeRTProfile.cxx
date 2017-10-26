#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>

#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TClonesArray.h>
#include <TF1.h>
#include <TH1.h>
#include <Math/Factory.h>
#include <Math/Functor.h>
#include <Math/Minimizer.h>

#include "GeomSvc.h"
#include "SRawEvent.h"
#include "FastTracklet.h"
#include "JobOptsSvc.h"

using namespace std;

const int nPointsMax = 500000;

int detectorID = 0;
int nPoints[25] = {0};
double cellWidth[25] = {0};
double tdcTime[25][nPointsMax] = {0};
double driftDistance[25][nPointsMax] = {0};
short  driftSign[25][nPointsMax] = {0};

bool acceptTracklet(Tracklet& tracklet)
{
    if(tracklet.getNHits() < 16) return false;
    if(tracklet.getProb() < 0.8) return false;
    if(1./tracklet.invP < 25.) return false;

    return true;
}

double t2r(int sign, double tdc, double t0, double p1, double p3, double rmax)
{
    double driftTime = t0 - tdc;
    if(driftTime < 0.) return 0.;

    driftTime = sign*driftTime;
    double r = p1*driftTime + p3*driftTime*driftTime*driftTime;
    if(r > rmax) r = rmax;

    return r;
}

double eval(const double* par)
{
    double chisq = 0.;
    for(int i = 0; i < nPoints[detectorID]; ++i)
    {
        double chi = driftSign[detectorID][i]*driftDistance[detectorID][i] - t2r(driftSign[detectorID][i], tdcTime[detectorID][i], par[0], par[1], par[2], cellWidth[detectorID]);
        chisq = chisq + chi*chi;
    }

    return chisq;
}

int main(int argc, char *argv[])
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    p_geomSvc->init(GEOMETRY_VERSION);
    for(int i = 1; i <= 24; ++i) cellWidth[i] = p_geomSvc->getCellWidth(i);

    //Input structure
    TClonesArray* tracklets = new TClonesArray("Tracklet");

    TFile* dataFile = new TFile(argv[1], "READ");
    TTree* dataTree = (TTree *)dataFile->Get("save");
    dataTree->SetBranchAddress("tracklets", &tracklets);

    int

    //Fill the data
    for(int i = 0; i < dataTree->GetEntries(); ++i)
    {
        dataTree->GetEntry(i);

        int nTracklets = tracklets->GetEntries();
        for(int j = 0; j < nTracklets; ++j)
        {
            Tracklet* track = (Tracklet*)tracklets->At(j);
            if(!acceptTracklet(*track)) continue;

            for(list<SignedHit>::iterator iter = track->hits.begin(); iter != track->hits.end(); ++iter)
            {
                if(iter->hit.index < 0) continue;

                int idx = iter->hit.detectorID;
                tdcTime[idx][nPoints[idx]] = iter->hit.tdcTime;
                driftSign[idx][nPoints[idx]] = iter->sign;
                driftDistance[idx][nPoints[idx]] = iter->sign*fabs(iter->hit.driftDistance) - track->residual[idx];

                ++nPoints[idx];
            }
        }
    }

    //Proceed to fit
    ROOT::Math::Minimizer* min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Combined");
    ROOT::Math::Functor fcn(&eval, 3);

    min->SetMaxFunctionCalls(1000000);
    min->SetMaxIterations(100000);
    min->SetTolerance(1E-3);
    min->SetFunction(fcn);
    min->SetPrintLevel(0);

    //Fit each plane
    for(int i = 1; i <= 24; ++i)
    {
        detectorID = i;

        double tmax = p_geomSvc->getPlane(detectorID).tmax;
        double tmin = p_geomSvc->getPlane(detectorID).tmin;
        min->SetLimitedVariable(0, "t0", tmax, 5., tmax - 50., tmax + 50.);
        min->SetLimitedVariable(1, "p1", cellWidth[detectorID]/(tmax - tmin), cellWidth[detectorID]/(tmax - tmin)*0.1, 0., cellWidth[detectorID]/(tmax - tmin)*5);
        min->SetLimitedVariable(2, "p3", 1E-8, 1E-9, 0, 1E-6);

        min->Minimize();
        cout << i << "  " << min->X()[0] << "  " << min->X()[1] << "  " << min->X()[2] << endl;
    }

    return 1;
}
