#ifndef _ONL_MON_HODO__H_
#define _ONL_MON_HODO__H_
#include "OnlMonClient.h"

class OnlMonHodo: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2, DP1T, DP1B, DP2T, DP2B } HodoType_t;
  static const int N_DET = 2;

 private:
  HodoType_t m_type;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  TH1* h1_ele     [N_DET];
  TH1* h1_ele_in  [N_DET];
  TH1* h1_time    [N_DET];
  TH1* h1_time_in [N_DET];
  TH2* h2_time_ele[N_DET];

 public:
  OnlMonHodo(const HodoType_t type);
  virtual ~OnlMonHodo() {}
  OnlMonClient* Clone() { return new OnlMonHodo(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void SetDet(const char* det0, const char* det1);
};

#endif /* _ONL_MON_HODO__H_ */
