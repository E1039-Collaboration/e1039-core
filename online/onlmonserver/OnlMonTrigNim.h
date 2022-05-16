#ifndef _ONL_MON_TRIG_NIM__H_
#define _ONL_MON_TRIG_NIM__H_
#include "OnlMonClient.h"
class SQHit;

class OnlMonTrigNim: public OnlMonClient {
  TH2* h2_count;

 public:
  OnlMonTrigNim();
  virtual ~OnlMonTrigNim() {}
  OnlMonClient* Clone() { return new OnlMonTrigNim(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_TRIG_NIM__H_ */
