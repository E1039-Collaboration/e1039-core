#include "GFMeasurement.h"

#include <iostream>
#include <iomanip>
#include <cmath>
#include <TVectorD.h>
#include <TMatrixDSym.h>
#include <TVector3.h>

#include <GenFit/KalmanFitterInfo.h>
#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/MeasurementOnPlane.h>
#include <GenFit/TrackPoint.h>
#include <GenFit/AbsHMatrix.h>

#include <geom_svc/GeomSvc.h>

namespace SQGenFit
{

GFMeasurement::GFMeasurement(SignedHit& rawHit, bool en)
{
  _bfHit = rawHit;

  GeomSvc* p_geomSvc = GeomSvc::instance();

  TVectorD ep1(3), ep2(3), hitcoord(7);
  int detID = _bfHit.hit.detectorID;
  p_geomSvc->getEndPoints(detID, _bfHit.hit.elementID, ep1, ep2);
  hitcoord[0] = ep1[0];
  hitcoord[1] = ep1[1];
  hitcoord[2] = ep1[2];
  hitcoord[3] = ep2[0];
  hitcoord[4] = ep2[1];
  hitcoord[5] = ep2[2];
  hitcoord[6] = fabs(_bfHit.hit.driftDistance);

  TMatrixDSym hitcov(7);
  double resol = p_geomSvc->getPlaneResolution(detID);
  hitcov.Zero();
  hitcov(6, 6) = resol*resol;

  //Base AbsMeasurement data
  setRawHitCoords(hitcoord);
  setRawHitCov(hitcov);
  setDetId(detID);
  setHitId(_bfHit.hit.index);
  setTrackPoint(nullptr);

  //WireMeasurement data
  setMaxDistance(0.5*p_geomSvc->getCellWidth(detID));
  setLeftRightResolution(0);

  //Own data
  _z = p_geomSvc->getPlanePosition(detID);
  _enableInFit = _bfHit.hit.index < 0 ? false : en;
  _proj = 0.;
  _driftSign = 0;
  _track = nullptr;
}

void GFMeasurement::setTrackPtr(GFTrack* trackPtr)
{
  _track = trackPtr;
}

void GFMeasurement::postFitUpdate()
{
  if(!_enableInFit) return;  //Needs extrapolation, not implemented for now

  genfit::KalmanFitterInfo* fitinfo = getTrackPoint()->getKalmanFitterInfo();
  const genfit::MeasuredStateOnPlane& state = fitinfo->getFittedState(false);  //biased = false since residual needs unbiased measured state
  genfit::MeasurementOnPlane* mstate = fitinfo->getClosestMeasurementOnPlane(&state);
  const genfit::AbsHMatrix* H = mstate->getHMatrix();

  _driftSign = mstate->getState()[0] > 0 ? 1 : -1;
  _proj = H->Hv(state.getState())[0];
}

void GFMeasurement::print(unsigned int debugLvl)
{
  std::cout << " ................................................" << std::endl;
  std::cout << "Hit " << _bfHit.hit.index << ", detID = " << _bfHit.hit.detectorID << ", eleID = " << _bfHit.hit.elementID 
            << ", pos = " << _bfHit.hit.pos << ", driftD = " << _bfHit.hit.driftDistance << ", pre-fit sign = " << _bfHit.sign
            << ", post-fit sign = " << _driftSign << ", projection = " << _proj << ", residual = " << _proj - _driftSign*_bfHit.hit.driftDistance << std::endl;

  if(debugLvl < 1) return;

  GeomSvc* p_geomSvc = GeomSvc::instance();
  TVector3 pos, mom;
  double w;

  genfit::KalmanFitterInfo* fitinfo = getTrackPoint()->getKalmanFitterInfo();
  const genfit::MeasuredStateOnPlane& fitstate = fitinfo->getFittedState(true);  //this time we need the biased/smoothed result
  fitstate.getPosMom(pos, mom);
  w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
  printHelper(w, pos, mom, "Fitted       ");
  if(debugLvl > 10) fitstate.getCov().Print();

  if(debugLvl > 1 && fitinfo->hasReferenceState())
  {
    genfit::StateOnPlane* state = static_cast<genfit::StateOnPlane*>(fitinfo->getReferenceState());

    state->getPosMom(pos, mom);
    w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
    printHelper(w, pos, mom, "Reference    ");
  }

  if(debugLvl > 2 && fitinfo->hasForwardPrediction())
  {
    genfit::MeasuredStateOnPlane* state = static_cast<genfit::MeasuredStateOnPlane*>(fitinfo->getForwardPrediction());

    state->getPosMom(pos, mom);
    w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
    printHelper(w, pos, mom, "F-prediction ");
    if(debugLvl > 10) state->getCov().Print();
  }

  if(debugLvl > 2 && fitinfo->hasForwardUpdate())
  {
    genfit::MeasuredStateOnPlane* state = static_cast<genfit::MeasuredStateOnPlane*>(fitinfo->getForwardUpdate());

    state->getPosMom(pos, mom);
    w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
    printHelper(w, pos, mom, "F-update     ");
    if(debugLvl > 10) state->getCov().Print();
  }

  if(debugLvl > 3 && fitinfo->hasBackwardPrediction())
  {
    genfit::MeasuredStateOnPlane* state = static_cast<genfit::MeasuredStateOnPlane*>(fitinfo->getBackwardPrediction());

    state->getPosMom(pos, mom);
    w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
    printHelper(w, pos, mom, "B-prediction ");
    if(debugLvl > 10) state->getCov().Print();
  }

  if(debugLvl > 3 && fitinfo->hasBackwardUpdate())
  {
    genfit::MeasuredStateOnPlane* state = static_cast<genfit::MeasuredStateOnPlane*>(fitinfo->getBackwardUpdate());

    state->getPosMom(pos, mom);
    w = p_geomSvc->getInterceptionFast(_bfHit.hit.detectorID, pos.X(), pos.Y());
    printHelper(w, pos, mom, "B-update     ");
    if(debugLvl > 10) state->getCov().Print();
  }
}

void GFMeasurement::printHelper(double w, TVector3& pos, TVector3& mom, TString name)
{
  std::cout << " -- " << name.Data() << ": ";
  std::cout << "pos (X,Y,Z,W) = " << std::setprecision(6) << pos.X() << " " << pos.Y() << " " << pos.Z() << " " << w;
  std::cout << " - mom(Px,Py,Pz) = " << mom.X() << " " << mom.Y() << " " << mom.Z();
  std::cout << std::endl;
}

}