#ifndef _GFMEASUREMENT_H
#define _GFMEASUREMENT_H

#include <GenFit/WireMeasurement.h>

#include "FastTracklet.h"

namespace SQGenFit
{

class GFTrack;

class GFMeasurement: public genfit::WireMeasurement
{
public:
  GFMeasurement(const SignedHit& rawHit, bool en = true);
  virtual ~GFMeasurement() {}

  void setTrackPtr(GFTrack* trackPtr);

  void enableHitInFit(bool en = true) { _enableInFit = en; }
  SignedHit& getBeforeFitHit() { return _bfHit; }
  void postFitUpdate();

  double getZ() { return _z; }
  bool isEnabled() { return _enableInFit; }

  void print(unsigned int debugLvl = 0);
  void printHelper(double w, TVector3& pos, TVector3& mom, TString name = "none");

private:
  SignedHit _bfHit;
  double _z;
  bool _enableInFit;

  double _proj;
  int _driftSign;
  GFTrack* _track;

};
}

#endif