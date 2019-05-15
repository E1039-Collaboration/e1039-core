#ifndef _ONL_MON_HODO__H_
#define _ONL_MON_HODO__H_
#include "OnlMonClient.h"
class SQSpill;
class SQEvent;
class SQHitVector;

class OnlMonHodo: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;

 private:
  HodoType_t m_type;
  int m_pl0;
  int m_n_pl;
  TH1* h1_ele    [99];
  TH1* h1_time   [99];
  TH1* h1_time_in[99];

 public:
  OnlMonHodo(const HodoType_t type);
  virtual ~OnlMonHodo() {}

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_HODO__H_ */
