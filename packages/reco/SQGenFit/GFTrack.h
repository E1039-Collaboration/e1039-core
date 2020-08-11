#ifndef _GFTRACK_H
#define _GFTRACK_H

#include <vector>
#include <memory>

#include <GenFit/AbsTrackRep.h>
#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/Track.h>
#include <GenFit/Tools.h>

#include <TString.h>
#include <TVector3.h>

#include "SRecEvent.h"
#include "FastTracklet.h"
#include "GFMeasurement.h"

namespace SQGenFit
{
class GFTrack
{
public:
  GFTrack();
  GFTrack(SRecTrack& recTrack);
  GFTrack(Tracklet& tracklet);
  ~GFTrack();

  void setVerbosity(unsigned int v);
  void setTracklet(Tracklet& tracklet, double z_reference = 590., bool wildseedcov = false);
  void addMeasurements(std::vector<GFMeasurement*>& measurements);
  void addMeasurement(GFMeasurement* measurement);

  double getChi2();
  double getNDF();
  int getCharge() { return _trkcand->getCharge(); };
  int getNearestMeasurementID(GFMeasurement* meas);

  //The extrapolation is implemented for line, plane and point, but the update/filter is only implemeted 
  // for line and plane, user needs to be careful and pass correct measurement and cov to get sensible result
  double extrapolateToLine(TVector3& endPoint1, TVector3& endPoint2, const int startPtID = 0);
  double extrapolateToPlane(TVector3& pO, TVector3& pU, TVector3& pV, const int startPtID = 0);
  double extrapolateToPoint(TVector3& point, bool update = false, const int startPtID = 0);
  double updatePropState(const TVectorD& meas, const TMatrixDSym& V);
  void getExtrapPosMomCov(TVector3& pos, TVector3& mom, TMatrixDSym& cov) { _propState->getPosMomCov(pos, mom, cov); }
  void getExtrapPosMom(TVector3& pos, TVector3& mom) { _propState->getPosMom(pos, mom); }

  double swimToVertex(double z, TVector3* pos = nullptr, TVector3* mom = nullptr, TMatrixDSym* cov = nullptr);

  void checkConsistency()  { _track->checkConsistency(); }

  void postFitUpdate(bool updateMeasurements = true);
  SRecTrack getSRecTrack();
  
  void print(unsigned int debugLvl = 0);

  genfit::Track* getGenFitTrack() { return _track; }
  genfit::AbsTrackRep* getGenFitTrkRep() { return _trkrep; }

private:
  //Auxilary function to initialize the extrapolation/projection, should not be seen by external user
  bool setInitialStateForExtrap(const int startPtID = 0);

  //interface with genfit operations
  genfit::AbsTrackRep* _trkrep; 
  genfit::Track* _track;
  std::vector<GFMeasurement*> _measurements;

  //container of the MSOP obtained from the SRecTrack
  std::vector<genfit::MeasuredStateOnPlane> _fitstates;

  //Used for the propagation and hypothesis test
  std::unique_ptr<genfit::MeasuredStateOnPlane> _propState;
  std::unique_ptr<genfit::AbsMeasurement> _virtMeas;

  //Store the original track candidate information from track finding
  Tracklet* _trkcand;
  int _pdg;

};
}

#endif