#ifndef __CALIB_IN_TIME_H__
#define __CALIB_IN_TIME_H__
#include <fun4all/SubsysReco.h>
class CalibParamInTimeTaiwan;
class CalibParamInTimeV1495;

class CalibInTime: public SubsysReco {
 public:
  CalibInTime(const std::string &name = "CalibInTime");
  virtual ~CalibInTime();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 private:
  CalibParamInTimeTaiwan* m_cal_taiwan;
  CalibParamInTimeV1495 * m_cal_v1495;
};

#endif /* __CALIB_IN_TIME_H__ */
