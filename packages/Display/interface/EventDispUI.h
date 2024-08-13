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
 protected:
  static const int RUN_MIN =  1000; //< Min of search range
  static const int RUN_MAX = 40000; //< Max of search range
  typedef std::vector<int> RunList_t;
  typedef std::vector<int> SpillList_t;
  RunList_t m_list_run;
  SpillList_t m_list_spill;

  int m_run;
  int m_spill;
  int m_n_evt;
  int m_i_evt;
  
  TGMainFrame* m_fr_main;
  TGCompositeFrame* m_fr_menu;
  TGCompositeFrame* m_fr_evt_mode;
  TGCompositeFrame* m_fr_evt_sel;
  TGCompositeFrame* m_fr_evt_next;
  TGCompositeFrame* m_fr_evt_prev;

  TGLabel* m_lbl_mode;
  TGLabel* m_lbl_run;
  TGLabel* m_lbl_n_evt;
  TGNumberEntry *m_ne_evt_id;
  TGNumberEntry *m_ne_trig;

  bool m_online_mode;
  bool m_auto_mode;
  pthread_t m_tid1;

 public:
  EventDispUI();
  virtual ~EventDispUI() {;}

  std::string GetDstPath(const int run, const int spill=0);
  bool FindNewRuns();
  bool FindSpillDSTs();

  int FetchNumEvents(const int run, const int spill=0);
  int OpenRunFile(const int run, const int spill=0);
  virtual void NextEvent();
  virtual void PrevEvent();
  void MoveEvent(const int i_evt);
  void ReqEvtID();
  void ReqTrig();
  void ViewTop ();
  void ViewSide();
  void View3D  ();
  void UpdateLabels();
  void SetAutoMode(bool value);
  void Init(const bool online_mode);
  virtual void Run();

 protected:
  void BuildInterface();
  void UpdateInterface();

  static void* FuncNewEventCheck(void* arg);
  void ExecNewEventCheck();

  int ReadNumber(const std::string msg="Run?");
};

#endif /* _EVENT_DISP_UI__H_ */
