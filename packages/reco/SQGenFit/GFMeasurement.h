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
  GFMeasurement(const GFMeasurement& meas);
  virtual ~GFMeasurement() {}

  virtual genfit::AbsMeasurement* clone() const override { return new GFMeasurement(*this); }

  void setTrackPtr(GFTrack* trackPtr);

  void enableHitInFit(bool en = true) { _enableInFit = en; }
  SignedHit& getBeforeFitHit() { return _bfHit; }
  void postFitUpdate();

  double getZ()    const { return _z; }
  bool isEnabled() const { return _enableInFit; }

  void print(unsigned int debugLvl = 0) const;
  
private:
  void printHelper(double w, TVector3& pos, TVector3& mom, TString name = "none") const;

  SignedHit _bfHit;
  double _z;
  bool _enableInFit;

  double _proj;
  int _driftSign;
  GFTrack* _track;

};

class GFMeasurementComp
{
public:
  bool operator() (const GFMeasurement* lhs, const GFMeasurement* rhs) const { return lhs->getZ() < rhs->getZ(); }
};

}

#endif