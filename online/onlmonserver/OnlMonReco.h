#ifndef _ONL_MON_RECO__H_
#define _ONL_MON_RECO__H_
#include "OnlMonClient.h"

class OnlMonReco: public OnlMonClient {
 public:

 private:
  TH1* h1_rec_stats;
  TH1* h1_sgmu_pt;
  TH1* h1_dimu_mass;

 public:
  OnlMonReco();
  virtual ~OnlMonReco() {}

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_RECO__H_ */
