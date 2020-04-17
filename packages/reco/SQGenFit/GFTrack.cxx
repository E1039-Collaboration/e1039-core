#include "GFTrack.h"

#include <iostream>
#include <algorithm>

#include <TVectorD.h>
#include <TMatrixDSym.h>

#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/MeasurementOnPlane.h>
#include <GenFit/WireMeasurement.h>
#include <GenFit/RKTrackRep.h>
#include <GenFit/AbsFitterInfo.h>
#include <GenFit/KalmanFitterInfo.h>
#include <GenFit/StateOnPlane.h>

#include <ktracker/SRawEvent.h>
#include <ktracker/SRecEvent.h>


namespace SQGenFit
{

GFTrack::GFTrack()
{
  _track  = nullptr;
  _trkrep = nullptr;
}

GFTrack::~GFTrack()
{
  if(_track != nullptr) delete _track;
}

void GFTrack::setVerbosity(unsigned int v)
{
  if(_trkrep != nullptr) _trkrep->setDebugLvl(v);
}

void GFTrack::addMeasurements(std::vector<GFMeasurement*>& measurements)
{
  for(auto iter = measurements.begin(); iter != measurements.end(); ++iter)
  {
    if(!(*iter)->isEnabled()) continue;
    addMeasurement(*iter);
  }
}

void GFTrack::addMeasurement(GFMeasurement* measurement)
{
  measurement->setTrackPtr(this);

  genfit::TrackPoint* tp = new genfit::TrackPoint(measurement, _track);
  tp->setSortingParameter(measurement->getZ());
  _track->insertPoint(tp);
}

double GFTrack::getChi2()
{
  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  if(rep)
  {
    genfit::FitStatus* fs = _track->getFitStatus(rep);
    if(fs) return fs->getChi2();
  }
  
  return -1.;
}

double GFTrack::getNDF()
{
  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  if(rep)
  {
    genfit::FitStatus* fs = _track->getFitStatus(rep);
    if(fs) return fs->getNdf();
  }
  
  return -1.;
}

int GFTrack::getNearestMeasurementID(GFMeasurement* meas)
{
  double z = meas->getZ();
  if(z < _measurements.front()->getZ())
  {
    return 0;
  }
  else if(z > _measurements.back()->getZ())
  {
    return _measurements.size() - 1;
  }
  else
  {
    auto iter = std::lower_bound(_measurements.begin(), _measurements.end(), meas, [](GFMeasurement* a, GFMeasurement* b) { return (a->getZ() < b->getZ()); });
    int idx = std::distance(_measurements.begin(), iter);
    return fabs(z - _measurements[idx]->getZ()) < fabs(z - _measurements[idx-1]->getZ()) ? idx : idx-1;
  }
}

double GFTrack::extrapolateToLine(genfit::MeasuredStateOnPlane& state, TVector3& endPoint1, TVector3& endPoint2, const int startPtID)
{
  TVector3 linePoint = (endPoint2 + endPoint1);
  TVector3 lineDir = endPoint2 - endPoint1;

  if(!setInitialStateForExtrap(state, startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  return rep->extrapolateToLine(state, linePoint, lineDir);
}

double GFTrack::extrapolateToPlane(genfit::MeasuredStateOnPlane& state, TVector3& pCenter, TVector3& pNorm, const int startPtID)
{
  genfit::SharedPlanePtr destPlane(new genfit::DetPlane(pCenter, pNorm));

  if(!setInitialStateForExtrap(state, startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  return rep->extrapolateToPlane(state, destPlane);
}

double GFTrack::extrapolateToPoint(genfit::MeasuredStateOnPlane& state, TVector3& point, const int startPtID)
{
  if(!setInitialStateForExtrap(state, startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  return rep->extrapolateToPoint(state, point);
}

bool GFTrack::setInitialStateForExtrap(genfit::MeasuredStateOnPlane& state, const int startPtID)
{
  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  genfit::TrackPoint*   tp = _track->getPointWithMeasurementAndFitterInfo(startPtID, rep);
  if(tp == nullptr) return false;

  genfit::KalmanFitterInfo* info = static_cast<genfit::KalmanFitterInfo*>(tp->getFitterInfo());
  state = info->getFittedState();

  return true;
}

void GFTrack::setTracklet(Tracklet& tracklet, bool wildseedcov)
{
  _trkcand = &tracklet;
  _pdg = tracklet.getCharge() > 0 ? -13 : 13;
  _trkrep = new genfit::RKTrackRep(_pdg);

  TVectorD seed_state(6);
  double z_st3 = 1900.;
  TVector3 seed_mom = tracklet.getMomentumSt3();
  TVector3 seed_pos(tracklet.getExpPositionX(z_st3), tracklet.getExpPositionY(z_st3), z_st3);
  seed_state[0] = seed_pos.X();
  seed_state[1] = seed_pos.Y();
  seed_state[2] = seed_pos.Z();
  seed_state[3] = seed_mom.Px();
  seed_state[4] = seed_mom.Py();
  seed_state[5] = seed_mom.Pz();

  TMatrixDSym seed_cov(6);
  for(int i = 0; i < 6; i++)
  {
    for(int j = 0; j < 6; j++)
    {
      seed_cov[i][j] = 100.;
    }
  }

  if(!wildseedcov)
  {
    //Implement a smaller cov based on the chi^2 fit?
  }
  _track = new genfit::Track(_trkrep, seed_state, seed_cov);

  _measurements.clear();
  for(auto iter = tracklet.hits.begin(); iter != tracklet.hits.end(); ++iter)
  {
    if(iter->hit.index < 0) continue;

    GFMeasurement* meas = new GFMeasurement(*iter);
    _measurements.push_back(meas);
  }
  std::sort(_measurements.begin(), _measurements.end(), [](GFMeasurement* a, GFMeasurement* b) { return (a->getZ() < b->getZ()); });

  addMeasurements(_measurements);
  checkConsistency();
}

void GFTrack::postFitUpdate(bool updateMeasurements)
{
  if(!updateMeasurements) return;
  for(auto iter = _measurements.begin(); iter != _measurements.end(); ++iter)
  {
    (*iter)->postFitUpdate();
  }
}

void GFTrack::print(unsigned int debugLvl)
{
  std::cout << "=============== SGTrack ===============" << std::endl;
  std::cout << "------------ Track candidate ----------" << std::endl;
  _trkcand->print();

  std::cout << "------------ Fit status ---------------" << std::endl;
  _track->getFitStatus(_trkrep)->Print();

  std::cout << "------------ Fit Result ---------------" << std::endl;
  _track->getFittedState().Print();  //default to the first hit

  if(debugLvl < 1) return;
  for(auto iter = _measurements.begin(); iter != _measurements.end(); ++iter)
  {
    (*iter)->print(debugLvl-1);
  }

  if(debugLvl < 20) return;
  std::cout << "------------ GenFit Track ---------------" << std::endl;
  _track->Print();
}

}
