#ifndef _EVENT_DISP_OFFLINE_UI__H_
#define _EVENT_DISP_OFFLINE_UI__H_
#include <vector>
#include "EventDispUI.h"

class EventDispOfflineUI : public EventDispUI {
 protected:
  std::string m_fn_dst;
  //int m_run;
  //int m_spill;
  //int m_n_evt;
  //int m_i_evt;
  //
  //TGNumberEntry *m_ne_evt_id;
  //TGNumberEntry *m_ne_trig;

 public:
  EventDispOfflineUI();
  virtual ~EventDispOfflineUI() {;}

  int FetchNumEvents(const std::string fn_dst);
  //virtual void NextEvent();
  virtual void PrevEvent();
  //void MoveEvent(const int i_evt);
  //void ReqEvtID();
  //void ReqTrig();
  //void ViewTop ();
  //void ViewSide();
  //void View3D  ();
  //void UpdateLabels();
  virtual void Run(const int run_id, const std::string fn_dst);

 protected:
  //void UpdateInterface();
};

#endif /* _EVENT_DISP_OFFLINE_UI__H_ */
