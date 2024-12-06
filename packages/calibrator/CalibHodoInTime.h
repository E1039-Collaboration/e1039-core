#ifndef __CALIB_HODO_IN_TIME_H__
#define __CALIB_HODO_IN_TIME_H__
#include <fun4all/SubsysReco.h>
class SQHitVector;
class CalibParamInTimeTaiwan;
class CalibParamInTimeV1495;

/// SubsysReco module to calibrate the in-time window of the hodoscope.
/**
 * The calibration parameter (i.e. center and width of window) is taken from DB, based on run number.
 * 
 * @code
 *   auto cal_hodo = new CalibHodoInTime();
 *   //cal_hodo->SkipCalibration(); // Uncomment this when needed.
 *   //cal_hodo->DeleteOutTimeHit(); // Uncomment this when needed.
 *   se->registerSubsystem(cal_hodo);
 * @endcode
 */
class CalibHodoInTime: public SubsysReco {
  bool m_skip_calib;
  bool m_delete_out_time_hit;
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

  /// Have this module skip the calibration.  Useful when you only delete out-of-time hits.
  void SkipCalibration() { m_skip_calib = true; }
  /// Have this module delete out-of-time hits.
  void DeleteOutTimeHit() { m_delete_out_time_hit = true; }
};

#endif // __CALIB_HODO_IN_TIME_H__
