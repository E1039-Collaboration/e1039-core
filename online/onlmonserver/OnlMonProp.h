#ifndef _ONL_MON_PROP__H_
#define _ONL_MON_PROP__H_
#include "OnlMonClient.h"

class OnlMonProp: public OnlMonClient {
 public:
  typedef enum { P1, P2 } PropType_t;
  static const int N_PL = 4;

 private:
  PropType_t m_type;
  int m_pl0;
  TH1* h1_ele [N_PL];
  TH1* h1_time[N_PL];

 public:
  OnlMonProp(const PropType_t type);
  virtual ~OnlMonProp() {}
  OnlMonClient* Clone() { return new OnlMonProp(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_PROP__H_ */
