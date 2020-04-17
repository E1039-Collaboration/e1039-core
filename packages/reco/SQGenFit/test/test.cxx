#include <iostream>
#include <cmath>
#include <list>

#include <TGeoManager.h>
#include <TRandom.h>

#include <jobopts_svc/JobOptsSvc.h>
#include <geom_svc/GeomSvc.h>
#include <phfield/PHField.h>

#include "GFField.h"
#include "GFFitter.h"
#include "GFTrack.h"
#include "ktracker/FastTracklet.h"
#include "ktracker/KalmanFastTracking.h"
#include "phool/recoConsts.h"

using namespace std;

int main()
{
  recoConsts* rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", 0.);
  rc->set_DoubleFlag("KMAGSTR", 0.);
  rc->Print();

  JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
  p_jobOptsSvc->init("run7_sim.opts");

  GeomSvc::UseDbSvc(true);
  GeomSvc* p_geomSvc = GeomSvc::instance();
  p_geomSvc->printTable();
  
  TGeoManager::Import("geom.root");

  //Let's hack a zero field
  PHField* phfield = nullptr;
  SQGenFit::GFField* gfield = new SQGenFit::GFField(phfield);
  gfield->disable();

  KalmanFastTracking* fasttracker = new KalmanFastTracking(phfield, gGeoManager, false);
  
  SQGenFit::GFFitter* fitter = new SQGenFit::GFFitter();
  fitter->init(gfield, "KalmanFitterRefTrack");
  fitter->setVerbosity(99);

  Tracklet candidate;
  candidate.stationID = 4;
  candidate.tx = -0.02;
  candidate.ty = -0.01;
  candidate.x0 = -5.;
  candidate.y0 = -5.;
  candidate.invP = 0.03;

  TRandom rndm(1);
  for(int i = 0; i < 12; ++i)
  {
    SignedHit newHit;

    newHit.hit.detectorID = 13+i;
    double w_exp = candidate.getExpPositionW(newHit.hit.detectorID);
    newHit.hit.elementID = p_geomSvc->getExpElementID(newHit.hit.detectorID, w_exp);
    newHit.hit.pos = p_geomSvc->getMeasurement(newHit.hit.detectorID, newHit.hit.elementID);
    newHit.hit.driftDistance = fabs(w_exp - newHit.hit.pos + rndm.Gaus(0., 1.*p_geomSvc->getPlaneResolution(newHit.hit.detectorID)));
    newHit.sign = w_exp > newHit.hit.pos ? 1 : -1;
    newHit.hit.index = i;

    candidate.hits.push_back(newHit);
  }
  candidate.print();
  fasttracker->fitTracklet(candidate);

  //cout << p_geomSvc->getInterception(19, 0.5, 0.5, 1.0, 1.0) << endl;
  candidate.print();

  SQGenFit::GFTrack gtrack;
  gtrack.setTracklet(candidate);
  gtrack.setVerbosity(99);

  cout << "----------------- Fit status: " << fitter->processTrack(gtrack) << endl;
  gtrack.postFitUpdate();
  //gtrack.print();

  /*
  genfit::MeasuredStateOnPlane msop1;
  TVector3 pc(0., 0., 0.);
  TVector3 pd(0., 0., 1.);
  double l = gtrack.extrapolateToPlane(msop1, pc, pd);
  cout << l << endl;
  msop1.Print();
  */

  cout << "----------------- Fit Results ------------------" << endl; 
  gtrack.getGenFitTrack()->Print();
  gtrack.print(10);

  //fitter->displayEvent();
  cout << "Exit" << endl;
  return 0;
}