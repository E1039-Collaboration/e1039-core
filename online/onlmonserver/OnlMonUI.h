#ifndef _ONL_MON_UI__H_
#define _ONL_MON_UI__H_
#include <vector>
class OnlMonClient;
class TGMainFrame;
class TGCompositeFrame;
class TGHorizontalFrame;
class TGTextButton;
class TGRadioButton;
class TGLabel;
class TGNumberEntry;
class TGDoubleSlider;

typedef std::vector<OnlMonClient*> OnlMonClientList_t;

class OnlMonUI {
  bool m_auto_cycle;
  int  m_interval; //< Cycle interval in second
  pthread_t m_tid1;
  pthread_t m_tid2;
  OnlMonClientList_t* m_list_omc;

  TGMainFrame* m_fr_main;
  TGCompositeFrame* m_fr_sp_range;

  TGRadioButton* m_rad_sp_all;
  TGRadioButton* m_rad_sp_last;
  TGRadioButton* m_rad_sp_range;

  TGLabel* m_lbl_sp;
  TGNumberEntry* m_num_sp;
  TGNumberEntry* m_num_sp0;
  TGNumberEntry* m_num_sp1;
  TGDoubleSlider* m_slider;

 public:
  OnlMonUI(OnlMonClientList_t* list);
  ~OnlMonUI() {;}

  void SetAutoCycleFlag(bool value) { m_auto_cycle = value; }
  bool GetAutoCycleFlag()    { return m_auto_cycle; }
  void SetCycleInterval(int val) { m_interval = val; }
  int  GetCycleInterval() { return m_interval; }
  void Run();

  void UpdateFullSpillRange();

  void HandleSpRadAll();
  void HandleSpRadLast();
  void HandleSpRadRange();
  void HandleSpLastNum();
  void HandleSpEntLo();
  void HandleSpEntHi();
  void HandleSpSlider();
  void SyncSpillRange();

 protected:
  void BuildInterface();
  void StartBgProc();
  static void* FuncAutoCycle(void* arg);
  void ExecAutoCycle();
  static void* ExecSpillRangeCheck(void* arg);

};

#endif /* _ONL_MON_UI__H_ */
