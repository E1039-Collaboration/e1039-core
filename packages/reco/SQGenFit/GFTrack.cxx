#include "GFTrack.h"

#include <iostream>
#include <algorithm>

#include <TVectorD.h>
#include <TMatrixDSym.h>

#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/MeasurementOnPlane.h>
#include <GenFit/WireMeasurement.h>
#include <GenFit/PlanarMeasurement.h>
#include <GenFit/RKTrackRep.h>
#include <GenFit/AbsFitterInfo.h>
#include <GenFit/KalmanFitterInfo.h>
#include <GenFit/StateOnPlane.h>

#include "SRawEvent.h"

namespace SQGenFit
{

GFTrack::GFTrack(): _track(nullptr), _trkrep(nullptr), _propState(nullptr), _virtMeas(nullptr), _trkcand(nullptr), _pdg(0)
{}

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

double GFTrack::extrapolateToLine(TVector3& endPoint1, TVector3& endPoint2, const int startPtID)
{
  TVector3 linePoint = (endPoint2 + endPoint1);
  TVector3 lineDir = endPoint2 - endPoint1;

  if(!setInitialStateForExtrap(startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  double len = rep->extrapolateToLine(*_propState, linePoint, lineDir);

  TVectorD hitcoord(7);
  hitcoord[0] = endPoint1.X();
  hitcoord[1] = endPoint1.Y();
  hitcoord[2] = endPoint1.Z();
  hitcoord[3] = endPoint2.X();
  hitcoord[4] = endPoint2.Y();
  hitcoord[5] = endPoint2.Z();
  hitcoord[6] = 0.;
  TMatrixDSym tempcov(7);
  tempcov.Zero();
  _virtMeas.reset(new genfit::WireMeasurement(hitcoord, tempcov, 999, 999, nullptr));

  return len;
}

double GFTrack::extrapolateToPlane(TVector3& pO, TVector3& pU, TVector3& pV, const int startPtID)
{
  genfit::SharedPlanePtr destPlane(new genfit::DetPlane(pO, pU, pV));

  if(!setInitialStateForExtrap(startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  double len = rep->extrapolateToPlane(*_propState, destPlane);

  TVectorD hitcoord(2);
  hitcoord[0] = pO.X();
  hitcoord[1] = pO.Y();
  TMatrixDSym tempcov(2);
  tempcov.UnitMatrix();

  genfit::PlanarMeasurement* pMeas = new genfit::PlanarMeasurement(hitcoord, tempcov, 998, 998, nullptr);
  pMeas->setPlane(destPlane);
  _virtMeas.reset(pMeas);

  return len;
}

double GFTrack::extrapolateToPoint(TVector3& point, bool update, const int startPtID)
{
  if(!setInitialStateForExtrap(startPtID)) return -9999.;

  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  double len = rep->extrapolateToPoint(*_propState, point);

  //TODO: implement the virtual spacepoint measurement here

  return len;
}

double GFTrack::updatePropState(const TVectorD& meas, const TMatrixDSym& V)
{
  //get the H matrix from measurement
  std::unique_ptr<const genfit::AbsHMatrix> H(_virtMeas->constructHMatrix(_propState->getRep()));

  //Handcrafted KF
  TVectorD stateVec(_propState->getState());
  TMatrixDSym cov(_propState->getCov());
  double chi2 = 9999.;
  double ndf = meas.GetNrows();

  TVectorD res(meas - H->Hv(stateVec));
  TMatrixDSym covSumInv(cov);
  H->HMHt(covSumInv);
  covSumInv += V;
  genfit::tools::invertMatrix(covSumInv);

  TMatrixD CHt(H->MHt(cov));
  TVectorD update(TMatrixD(CHt, TMatrixD::kMult, covSumInv)*res);

  stateVec += update;
  covSumInv.Similarity(CHt);
  cov -= covSumInv;
  _propState->setStateCov(stateVec, cov);

  //calc chi2
  TVectorD resNew(meas - H->Hv(stateVec));
  TMatrixDSym HCHt(cov);
  H->HMHt(HCHt);
  HCHt -= V;
  HCHt *= -1;
  genfit::tools::invertMatrix(HCHt);
  chi2 = HCHt.Similarity(resNew);

  return chi2/ndf;
}

bool GFTrack::setInitialStateForExtrap(const int startPtID)
{
  genfit::AbsTrackRep* rep = _track->getCardinalRep();
  genfit::TrackPoint*   tp = _track->getPointWithMeasurementAndFitterInfo(startPtID, rep);
  if(tp == nullptr) return false;

  genfit::KalmanFitterInfo* info = static_cast<genfit::KalmanFitterInfo*>(tp->getFitterInfo(rep));
  _propState.reset(new genfit::MeasuredStateOnPlane(info->getFittedState()));

  return true;
}

void GFTrack::setTracklet(Tracklet& tracklet, double z_reference, bool wildseedcov)
{
  _trkcand = &tracklet;
  _pdg = tracklet.getCharge() > 0 ? -13 : 13;
  _trkrep = new genfit::RKTrackRep(_pdg);

  TVectorD seed_state(6);
    
  TVector3 seed_mom = tracklet.getExpMomentum(z_reference);
  TVector3 seed_pos(tracklet.getExpPositionX(z_reference), tracklet.getExpPositionY(z_reference), z_reference);
  seed_state[0] = seed_pos.X();
  seed_state[1] = seed_pos.Y();
  seed_state[2] = seed_pos.Z();
  seed_state[3] = seed_mom.Px();
  seed_state[4] = seed_mom.Py();
  seed_state[5] = seed_mom.Pz();

  TMatrixDSym seed_cov(6);
  double uncertainty[6] = {10., 10., 10., 3., 3., 10.};
  for(int i = 0; i < 6; i++)
  {
    for(int j = 0; j < 6; j++)
    {
      seed_cov[i][j] = uncertainty[i]*uncertainty[j];
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

SRecTrack GFTrack::getSRecTrack()
{
  //postFitUpdate();
  //The following steps are pretty hacky and should be considered as only a temporary solution
  SRecTrack strack;
  strack.setChisq(getChi2());
  for(auto iter = _measurements.begin(); iter != _measurements.end(); ++iter)
  {
    strack.insertHitIndex((*iter)->getBeforeFitHit().hit.index);

    const genfit::MeasuredStateOnPlane& fitstate = (*iter)->getTrackPoint()->getKalmanFitterInfo()->getFittedState(true);
    
    TVector3 pos, mom;
    fitstate.getPosMom(pos, mom);
    TMatrixD stateVec(5, 1);
    stateVec[0][0] = getCharge()/mom.Mag();
    stateVec[1][0] = mom.Px()/mom.Pz();
    stateVec[2][0] = mom.Py()/mom.Pz();
    stateVec[3][0] = pos.X();
    stateVec[4][0] = pos.Y();
    strack.insertStateVector(stateVec);
    strack.insertZ(pos.Z());

    TMatrixD cov(fitstate.getCov());
    strack.insertCovariance(cov);
    
    strack.insertChisq((*iter)->getTrackPoint()->getKalmanFitterInfo()->getSmoothedChi2());
  }

  //Swim to various places and save info
  strack.swimToVertex(nullptr, nullptr, false);

  //Hypothesis test should be implemented here
  TVector3 pO(0., 0., Z_UPSTREAM);
  TVector3 pU(1., 0., 0.);
  TVector3 pV(0., 1., 0.);
  TVectorD beamCenter(2);
  beamCenter[0] = 0.; beamCenter[1] = 0.;
  TMatrixDSym beamCov(2);
  beamCov.Zero();
  beamCov(0, 0) = 100.; beamCov(1, 1) = 100.;

  //test Z_UPSTREAM
  try
  {
    double len = extrapolateToPlane(pO, pU, pV);
    if(fabs(len) > 6000.) throw len;
    strack.setChisqUpstream(updatePropState(beamCenter, beamCov));  
  }
  catch(genfit::Exception& e)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed upstream: " << e.what() << std::endl;
    strack.setChisqUpstream(999.);
  }
  catch(double len)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed upstream: " << len << std::endl;
    strack.setChisqUpstream(999.);
  }

  //test Z_TARGET
  try
  {
    pO.SetZ(Z_TARGET);
    double len = extrapolateToPlane(pO, pU, pV);
    if(fabs(len) > 6000.) throw len;
    strack.setChisqTarget(updatePropState(beamCenter, beamCov));
  }
  catch(genfit::Exception& e)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed target: " << e.what() << std::endl;
    strack.setChisqTarget(999.);
  }
  catch(double len)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed target: " << len << std::endl;
    strack.setChisqTarget(999.);
  }

  //test Z_DUMP
  try
  {
    pO.SetZ(Z_DUMP);
    double len = extrapolateToPlane(pO, pU, pV);
    if(fabs(len) > 6000.) throw len;
    strack.setChisqDump(updatePropState(beamCenter, beamCov));
  }
  catch(genfit::Exception& e)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed dump: " << e.what() << std::endl;
    strack.setChisqDump(999.);
  }
  catch(double len)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed dump: " << len << std::endl;
    strack.setChisqDump(999.);
  }

  /*
  //Find POCA to beamline -- it seems to be funky and mostly found some place way upstream or downstream
  // most likely because the cross product of the track direction and beam line direction is way too small 
  // z axis to provide reasonable calculation of the POCA location. It's disabled for now.
  TVector3 ep1(0., 0., -499.);
  TVector3 ep2(0., 0., 0.);
  try
  {
    extrapolateToLine(ep1, ep2);
    TVectorD beamR(1); beamR(0) = 0.;
    TMatrixDSym beamC(1); beamC(0, 0) = 1000.;
    strack.setChisqVertex(updatePropState(beamR, beamC));
  }
  catch(genfit::Exception& e)
  {
    std::cerr << "Hypothesis test failed at beamline: " << e.what() << std::endl;
    print(0);
  }
  */

  //test Z_VERTEX
  try
  {
    pO.SetZ(strack.getVertexPos().Z());
    double len = extrapolateToPlane(pO, pU, pV);
    if(fabs(len) > 6000.) throw len;
    strack.setChisqVertex(updatePropState(beamCenter, beamCov));
  }
  catch(genfit::Exception& e)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed vertex @Z=" << strack.getVertexPos().Z() << ": " << e.what() << std::endl;
    strack.setChisqVertex(999.);
  }
  catch(double len)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed vertex @Z=" << strack.getVertexPos().Z() << ": " << len << std::endl;
    strack.setChisqVertex(999.);
  }

  strack.setKalmanStatus(1);
  return strack;
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
