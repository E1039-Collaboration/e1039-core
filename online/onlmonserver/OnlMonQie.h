/// OnlMonQie
#ifndef _ONL_MON_QIE__H_
#define _ONL_MON_QIE__H_
#include "OnlMonClient.h"

class OnlMonQie: public OnlMonClient {
  static const int N_PRESUM = 4;
  static const int N_RF_INTE = 33;
  typedef enum {
    ALL = 1,
    OK  = 2,
    NO_DATA = 3,
    EXTRA_DATA = 4,
    TURN_RF_ID_ZERO = 5,
    ZERO_PM13 = 6,
    ZERO_PM08 = 7
  } EvtStatusIndex_t;

  TH1* h1_evt_status;
  //TH1* h1_trig_cnt;
  //TH2* h2_presum;
  TH2* h2_turn_rf;
  TH2* h2_inte_rf;

 public:
  OnlMonQie();
  virtual ~OnlMonQie() {}
  OnlMonClient* Clone() { return new OnlMonQie(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_QIE__H_ */
