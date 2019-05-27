#ifndef __CALIB_XT_H__
#define __CALIB_XT_H__
#include <fun4all/SubsysReco.h>
class CalibParamXT;
class CalibParamInTimeTaiwan;

class CalibXT: public SubsysReco {
 public:
  CalibXT(const std::string &name = "CalibXT");
  virtual ~CalibXT();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  std::string GetDBConf() { return m_db_conf; }
  void        SetDBConf(const std::string db_conf) { m_db_conf = db_conf; }

 private:
  CalibParamXT* m_cal_xt;
  CalibParamInTimeTaiwan* m_cal_int;

  std::string m_db_conf;
};

#endif /* __CALIB_XT_H__ */
