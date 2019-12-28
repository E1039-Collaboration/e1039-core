#ifndef _ANA_WAIT__H_
#define _ANA_WAIT__H_
#include <fun4all/SubsysReco.h>

class AnaWait: public SubsysReco {
  int m_wait_spill;
  int m_wait_event;

 public:
  AnaWait(const int sec_spill=30, const int sec_event=0);
  virtual ~AnaWait();

  void SetWaitPerSpill(const int sec) { m_wait_spill = sec; }
  void SetWaitPerEvent(const int sec) { m_wait_event = sec; }

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 protected:
  void DoWait(int sec);
};

#endif /* _ANA_WAIT__H_ */
