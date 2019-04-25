#ifndef __CALIB_PARAM_IN_TIME_V1495_H__
#define __CALIB_PARAM_IN_TIME_V1495_H__
#include "RunParamBase.h"

class CalibParamInTimeV1495 : public CalibParamBase {
  struct ParamItem {
    std::string det_name;
    short  det;
    short  ele;
    short  lvl;
    double center;
    double width;
  };
  typedef std::vector<ParamItem> List_t;
  List_t m_list; ///< Used to keep all information in the added order.

  typedef std::tuple<short, short, short> DetEleLvl_t;
  typedef std::pair<double, double> CenterWidth_t;
  typedef std::map<DetEleLvl_t, CenterWidth_t> Map_t;
  Map_t m_map; ///< Used in Find() for better speed.

 public:
  CalibParamInTimeV1495();
  virtual ~CalibParamInTimeV1495() {;}

  void Add(const std::string det     ,                     const short ele, const short lvl, const double center, const double width);
  void Add(const std::string det_name, const short det_id, const short ele, const short lvl, const double center, const double width);

  bool Find(const short det, const short ele, const short lvl, double& center, double& width);
  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CALIB_PARAM_IN_TIME_V1495_H__
