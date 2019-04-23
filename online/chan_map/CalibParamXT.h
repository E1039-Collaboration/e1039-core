#ifndef __CALIB_PARAM_XT_H__
#define __CALIB_PARAM_XT_H__
#include "RunParamBase.h"
class TGraphErrors;

class CalibParamXT : public CalibParamBase {
  struct ParamItem {
    std::string det_name;
    short  det;
    double t;
    double x;
    double dx;
  };
  typedef std::vector<ParamItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::map<short, TGraphErrors*> Map_t;
  Map_t m_map_t2x;
  Map_t m_map_t2dx;

 public:
  CalibParamXT();
  virtual ~CalibParamXT();

  void Add(const std::string det     ,                     const double t, const double x, const double dx);
  void Add(const std::string det_name, const short det_id, const double t, const double x, const double dx);

  bool Find(const short det, TGraphErrors*& gr_t2x, TGraphErrors*& gr_t2dx);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CALIB_PARAM_XT_H__
