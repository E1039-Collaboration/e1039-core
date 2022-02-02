#ifndef __CALIB_PARAM_XT_H__
#define __CALIB_PARAM_XT_H__
#include <TGraphErrors.h>
#include "RunParamBase.h"
class TGraph;
class TGraphErrors;

/// Calibration parameter for chamber X-T relation.
/**
 * The parameter 't' means the TDC time, where the meaning was changed from the drift time on 2022-01-28.
 * This class carries the in-time range of the chambers, since the minimum and maximum of 't' are T1 and T0.
 */
class CalibParamXT : public CalibParamBase {
 public:
  /// A set of parameters for one detector (plane).
  struct Set {
    double X0; ///< Minimum X.  Usually zero.
    double X1; ///< Maximim X.  Half cell width.
    double T0; ///< T at X0.  T0 > T1.
    double T1; ///< T at X1.
    TGraphErrors t2x;
    TGraph       t2dx;
    TGraph       t2dt;
    TGraphErrors x2t;
    TGraph       x2dt;
    TGraph       x2dx;
    void Add(const double t, const double x, const double dt, const double dx);
  };

 private:
  /// A parameter item in TSV or MySQL DB.
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

  typedef std::map<short, Set> SetMap_t; // [det_id] = Set
  SetMap_t m_map_sets;

 public:
  CalibParamXT();
  virtual ~CalibParamXT();

  void Add(const std::string det     ,                     const double t, const double x, const double dt, const double dx);
  void Add(const std::string det_name, const short det_id, const double t, const double x, const double dt, const double dx);

  Set* GetParam(const short det); ///< Return a set of parameters for `det`.  Return 0 if `det` is invalid.

  void Print(std::ostream& os);

 protected:
  int  ReadFileCont(LineList& lines);
  int WriteFileCont(std::ostream& os);

  void  ReadDbTable(DbSvc& db);
  void WriteDbTable(DbSvc& db);
};

#endif // __CALIB_PARAM_XT_H__
