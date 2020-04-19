#include <iostream>
#include <cmath>
#include <list>

#include <TGeoManager.h>
#include <TRandom.h>

#include <jobopts_svc/JobOptsSvc.h>
#include <geom_svc/GeomSvc.h>
#include <phfield/PHField.h>
#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHFieldUtility.h>
#include <phool/recoConsts.h>

#include "GFField.h"
#include "GFFitter.h"
#include "GFTrack.h"
#include "FastTracklet.h"
#include "KalmanFastTracking.h"

using namespace std;

int main()
{
  recoConsts* rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", 1.);
  rc->set_DoubleFlag("KMAGSTR", 1.);
  rc->Print();

  JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
  p_jobOptsSvc->init("run7_sim.opts");

  GeomSvc::UseDbSvc(true);
  GeomSvc* p_geomSvc = GeomSvc::instance();
  p_geomSvc->printTable();
  
  TGeoManager::Import("geom.root");

  //Let's hack a zero field
  unique_ptr<PHFieldConfig> default_field_cfg(nullptr);
	default_field_cfg.reset(new PHFieldConfig_v3(p_jobOptsSvc->m_fMagFile, p_jobOptsSvc->m_kMagFile));
  PHField* phfield = PHFieldUtility::BuildFieldMap(default_field_cfg.get(), 10);
  SQGenFit::GFField* gfield = new SQGenFit::GFField(phfield);

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