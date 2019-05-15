#ifndef _ONL_MON_TRIG_SIG__H_
#define _ONL_MON_TRIG_SIG__H_
#include "OnlMonClient.h"
class SQSpill;
class SQEvent;
class SQHitVector;

class OnlMonTrigSig: public OnlMonClient {
  TH2* h2_bi_fpga;
  TH2* h2_ai_fpga;
  TH2* h2_bi_nim;
  TH2* h2_ai_nim;

 public:
  OnlMonTrigSig();
  virtual ~OnlMonTrigSig() {}

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_TRIG_SIG__H_ */
