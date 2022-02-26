#ifndef __DECO_ERROR_H__
#define __DECO_ERROR_H__
#include <vector>
//#include "assert.h"
class DbSvc;

/** This class manages errors found in the decoder.
 * At present the decoder error is aggregated once per spill.
 * Therefore
 *  - InitData() is called at EOS.
 *  - Errors are recorded during flush events are decoded.
 *  - UploadToDB() is called at BOS.
 */
class DecoError {
 public:
  typedef enum {
    WORD_ONLY89     = 0,
    WORD_OVERFLOW   = 1,
    MULTIPLE_HEADER = 2,
    EVT_ID_ONLY     = 3,
    START_WO_STOP   = 4,
    START_NOT_RISE  = 5,
    DIRTY_FINISH    = 6,
    N_TDC_ERROR     = 7,
    V1495_0BAD      = 8
  } TdcError_t;

 private:
  static const int N_ROC = 33;

  int m_run_id;
  int m_spill_id;
  bool m_flush_has_error;

  int m_n_evt_all;
  int m_n_evt_ng;
  std::vector<int> m_n_err_tdc[N_ROC][N_TDC_ERROR];

 public:
  DecoError();
  ~DecoError() {;}

  void SetID(const int run_id, const int spill_id);
  void SetFlushError(const bool val) { m_flush_has_error = val; }
  bool GetFlushError()        { return m_flush_has_error; }

  void InitData();
  void CountFlush();
  void AddTdcError(const int event, const int roc, const TdcError_t type);
  void AggregateData();
  void PrintData(std::ostream& os=std::cout);

 private:
  void UpdateDbInfo(DbSvc* db);
  void UpdateDbTdc (DbSvc* db);
};

#endif // __DECO_ERROR_H__
