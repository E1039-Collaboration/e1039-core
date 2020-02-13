#ifndef _OUTPUT_EVT_DISP_FILE_H_
#define _OUTPUT_EVT_DISP_FILE_H_
#include <fun4all/SubsysReco.h>
class SQEvent;
class SQHitVector;

class EvtDispFilter: public SubsysReco {
  int m_n_step; // N of skipped events per one saved event
  int m_n_max; // N of max events saved per spill
  SQEvent*     m_sq_evt;
  SQHitVector* m_sq_hv;
  
 public:
  EvtDispFilter(const int n_step=1000, const int n_max=3);
  virtual ~EvtDispFilter();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
};

#endif /* _OUTPUT_EVT_DISP_FILE_H_ */
