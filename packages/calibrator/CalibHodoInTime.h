#ifndef __CALIB_HODO_IN_TIME_H__
#define __CALIB_HODO_IN_TIME_H__
#include <fun4all/SubsysReco.h>
class SQHitVector;
class CalibParamInTimeTaiwan;
class CalibParamInTimeV1495;

/// SubsysReco module to calibrate the in-time window of the hodoscope.
/**
 * It does _not_ take care of the DP hodoscope at present.
 */
class CalibHodoInTime: public SubsysReco {
  SQHitVector* m_vec_hit;
  SQHitVector* m_vec_trhit;
  CalibParamInTimeTaiwan* m_cal_taiwan;
  CalibParamInTimeV1495 * m_cal_v1495;

 public:
  CalibHodoInTime(const std::string &name = "CalibHodoInTime");
  virtual ~CalibHodoInTime();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif // __CALIB_HODO_IN_TIME_H__
