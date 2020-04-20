#ifndef _GFTRACK_H
#define _GFTRACK_H

#include <vector>

#include <GenFit/AbsTrackRep.h>
#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/Track.h>

#include <TString.h>
#include <TVector3.h>

#include "FastTracklet.h"
#include "GFMeasurement.h"

namespace SQGenFit
{
class GFTrack
{
public:
  GFTrack();
  ~GFTrack();

  void setVerbosity(unsigned int v);
  void setTracklet(Tracklet& tracklet, double z_reference = 590., bool wildseedcov = false);
  void addMeasurements(std::vector<GFMeasurement*>& measurements);
  void addMeasurement(GFMeasurement* measurement);

  double getChi2();
  double getNDF();
  int getCharge() { return _trkcand->getCharge(); };
  int getNearestMeasurementID(GFMeasurement* meas);

  double extrapolateToLine(genfit::MeasuredStateOnPlane& state, TVector3& endPoint1, TVector3& endPoint2, const int startPtID = 0);
  double extrapolateToPlane(genfit::MeasuredStateOnPlane& state, TVector3& pCenter, TVector3& pNorm, const int startPtID = 0);
  double extrapolateToPoint(genfit::MeasuredStateOnPlane& state, TVector3& point, const int startPtID = 0);
  bool setInitialStateForExtrap(genfit::MeasuredStateOnPlane& state, const int startPtID = 0);

  void checkConsistency()  { _track->checkConsistency(); }

  void postFitUpdate(bool updateMeasurements = true);
  
  void print(unsigned int debugLvl = 0);

  genfit::Track* getGenFitTrack() { return _track; }

private:
  genfit::AbsTrackRep* _trkrep; 
  genfit::Track* _track;

  std::vector<GFMeasurement*> _measurements;

  Tracklet* _trkcand;
  int _pdg;

};
}

#endif