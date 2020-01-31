#ifndef _EVENT_DISP_UI__H_
#define _EVENT_DISP_UI__H_
#include <vector>
class TGMainFrame;
class TGCompositeFrame;
class TGHorizontalFrame;
class TGTextButton;
class TGRadioButton;
class TGLabel;
class TGNumberEntry;

class EventDispUI {
  static const int RUN_MIN = 1000; //< Min of search range
  static const int RUN_MAX = 4000; //< Max of search range
  typedef std::vector<int> RunList_t;
  RunList_t m_list_run;

  int m_run;
  int m_n_evt;
  int m_i_evt;
  
  TGMainFrame* m_fr_main;
  TGCompositeFrame* m_fr_menu;
  TGCompositeFrame* m_fr_evt_nav;

  TGLabel* m_lbl_run;
  TGLabel* m_lbl_n_evt;
  TGNumberEntry *m_ne_evt_id;
  TGNumberEntry *m_ne_trig;
  
  bool m_auto_mode;
  pthread_t m_tid1;

 public:
  EventDispUI(const bool auto_mode=false);
  ~EventDispUI() {;}

  std::string GetDstPath(const int run);
  bool FindNewRuns();

  int FetchNumEvents(const int run);
  int OpenRunFile(const int run);
  void NextEvent();
  void PrevEvent();
  void MoveEvent(const int i_evt);
  void ReqEvtID();
  void ReqTrig();
  void ViewTop ();
  void ViewSide();
  void View3D  ();
  void UpdateLabels();
  void SetAutoMode(bool value);
  void Run();

 protected:
  void BuildInterface();

  static void* FuncNewEventCheck(void* arg);
  void ExecNewEventCheck();
};

#endif /* _EVENT_DISP_UI__H_ */
