/// OnlMonMainDaq
#ifndef _ONL_MON_MAIN_DAQ__H_
#define _ONL_MON_MAIN_DAQ__H_
#include "OnlMonClient.h"
class SQEvent;
class TArrayI;

class OnlMonMainDaq: public OnlMonClient {
  TH1* h1_trig;
  TH1* h1_evt_qual;
  TH1* h1_flag_v1495;
  TH1* h1_cnt;

 public:
  OnlMonMainDaq();
  virtual ~OnlMonMainDaq() {}

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  //void PrintEvent(SQEvent* evt, SQHitVector* v_hit, SQHitVector* v_trig_hit);
};

#endif /* _ONL_MON_MAIN_DAQ__H_ */
