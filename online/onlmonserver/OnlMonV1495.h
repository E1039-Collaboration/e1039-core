#ifndef _ONL_MON_V1495__H_
#define _ONL_MON_V1495__H_
#include "OnlMonClient.h"
class SQSpill;
class SQEvent;
class SQHitVector;

class OnlMonV1495: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;

 private:
  HodoType_t m_type;
  int m_lvl;
  int m_pl0;
  int m_n_pl;
  TH1* h1_ele    [99];
  TH1* h1_ele_in [99];
  TH1* h1_time   [99];
  TH1* h1_time_in[99];

 public:
  OnlMonV1495(const HodoType_t type, const int lvl);
  virtual ~OnlMonV1495() {}

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_V1495__H_ */
