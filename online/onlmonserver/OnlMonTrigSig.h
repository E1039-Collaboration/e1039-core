#ifndef _ONL_MON_TRIG_SIG__H_
#define _ONL_MON_TRIG_SIG__H_
#include "OnlMonClient.h"

class OnlMonTrigSig: public OnlMonClient {
  TH2* h2_bi_fpga;
  TH2* h2_ai_fpga;
  TH2* h2_bi_nim;
  TH2* h2_ai_nim;
  TH2* h2_rf;
  TH2* h2_stop;

 public:
  OnlMonTrigSig();
  virtual ~OnlMonTrigSig() {}
  OnlMonClient* Clone() { return new OnlMonTrigSig(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void DrawTH2WithPeakPos(TH2* h2, const double cont_min=100);
};

#endif /* _ONL_MON_TRIG_SIG__H_ */
