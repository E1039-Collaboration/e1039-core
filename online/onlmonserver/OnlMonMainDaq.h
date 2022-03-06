/// OnlMonMainDaq
#ifndef _ONL_MON_MAIN_DAQ__H_
#define _ONL_MON_MAIN_DAQ__H_
#include "OnlMonClient.h"

class OnlMonMainDaq: public OnlMonClient {
  unsigned int m_spill_id_1st;
  TH1* h1_trig;
  TH1* h1_n_taiwan;
  TH1* h1_evt_qual;
  TH1* h1_flag_v1495;
  TH1* h1_cnt;
  TH1* h1_nevt_sp;
  TH2* h2_nhit_pl;

 public:
  OnlMonMainDaq();
  virtual ~OnlMonMainDaq() {}
  OnlMonClient* Clone() { return new OnlMonMainDaq(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_MAIN_DAQ__H_ */
