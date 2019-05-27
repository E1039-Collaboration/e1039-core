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

  std::string GetDBConf() { return m_db_conf; }
  void        SetDBConf(const std::string db_conf) { m_db_conf = db_conf; }

 private:
  CalibParamInTimeTaiwan* m_cal_taiwan;
  CalibParamInTimeV1495 * m_cal_v1495;

  std::string m_db_conf;
};

#endif /* __CALIB_IN_TIME_H__ */
