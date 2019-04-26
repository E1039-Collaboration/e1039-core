/// OnlMonMainDaq
#ifndef _ONL_MON_MAIN_DAQ__H_
#define _ONL_MON_MAIN_DAQ__H_
#include "OnlMonClient.h"
class SQSpill;
class SQEvent;
class SQHitVector;

class OnlMonMainDaq: public OnlMonClient {
  TH1* h1_tgt;
  TH1* h1_evt_qual;

 public:
  OnlMonMainDaq(const std::string &name = "OnlMonMainDaq");
  virtual ~OnlMonMainDaq() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  int DrawMonitor();

 private:
  void PrintEvent(SQEvent* evt, SQHitVector* v_hit, SQHitVector* v_trig_hit);
};

#endif /* _ONL_MON_MAIN_DAQ__H_ */
