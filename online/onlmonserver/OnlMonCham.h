#ifndef _ONL_MON_CHAM__H_
#define _ONL_MON_CHAM__H_
#include "OnlMonClient.h"

class OnlMonCham: public OnlMonClient {
 public:
  typedef enum { D0, D1, D2, D3p, D3m } ChamType_t;
  static const int N_PL = 6;

 private:
  ChamType_t m_type;
  int m_pl0;
  TH1* h1_ele [N_PL];
  TH1* h1_time[N_PL];

 public:
  OnlMonCham(const ChamType_t type);
  virtual ~OnlMonCham() {}
  OnlMonClient* Clone() { return new OnlMonCham(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_CHAM__H_ */
