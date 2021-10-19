#ifndef __CALIB_DRIFT_DIST_H__
#define __CALIB_DRIFT_DIST_H__
#include <fun4all/SubsysReco.h>
class SQHitVector;
class CalibParamXT;
class CalibParamInTimeTaiwan;

/// SubsysReco module to calibrate the drift distance and also the in-time window of the chambers and the prop tube.
class CalibDriftDist: public SubsysReco {
  SQHitVector* m_vec_hit;
  CalibParamXT* m_cal_xt;
  CalibParamInTimeTaiwan* m_cal_int;

 public:
  CalibDriftDist(const std::string &name = "CalibDriftDist");
  virtual ~CalibDriftDist();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif // __CALIB_DRIFT_DIST_H__
