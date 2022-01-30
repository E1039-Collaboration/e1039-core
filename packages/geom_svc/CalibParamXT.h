#ifndef __CALIB_PARAM_XT_H__
#define __CALIB_PARAM_XT_H__
#include "RunParamBase.h"
class TGraph;
class TGraphErrors;

/// Calibration parameter for chamber X-T relation.
/**
 * The parameter 't' means the TDC time, where the meaning was changed from the drift time on 2022-01-28.
 * This class carries the in-time range of the chambers, since the minimum and maximum of 't' are T1 and T0.
 */
class CalibParamXT : public CalibParamBase {
  struct ParamItem {
    std::string det_name;
    short  det;
    double t;
    double x;
    double dt;
    double dx;
  };
  typedef std::vector<ParamItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::map<short, TGraphErrors*> Map_t;
  Map_t m_map_t2x;
  Map_t m_map_t2dx;
  Map_t m_map_x2t;
  Map_t m_map_x2dt;

 public:
  CalibParamXT();
  virtual ~CalibParamXT();

  void Add(const std::string det     ,                     const double t, const double x, const double dt, const double dx);
  void Add(const std::string det_name, const short det_id, const double t, const double x, const double dt, const double dx);

  bool FindT2X(const short det, TGraphErrors*& gr_t2x, TGraphErrors*& gr_t2dx);
  bool FindX2T(const short det, TGraphErrors*& gr_x2t, TGraphErrors*& gr_x2dt);
  void Print(std::ostream& os);

  static bool FindT1T0FromT2X(const TGraph* gr_t2x, double& t1, double &t0);
  static bool FindT1T0FromX2T(const TGraph* gr_x2t, double& t1, double &t0);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CALIB_PARAM_XT_H__
