#ifndef _ONL_MON_UI__H_
#define _ONL_MON_UI__H_
#include <vector>
class OnlMonClient;
//class TGTextButton;

typedef std::vector<OnlMonClient*> OnlMonClientList_t;

class OnlMonUI {
  bool m_auto_cycle;
  int  m_interval; //< Cycle interval in second
  pthread_t m_thread_id;
  OnlMonClientList_t* m_list_omc;

 public:
  OnlMonUI(OnlMonClientList_t* list);
  ~OnlMonUI() {;}

  void SetAutoCycleFlag(bool value) { m_auto_cycle = value; }
  bool GetAutoCycleFlag()    { return m_auto_cycle; }
  void SetCycleInterval(int val) { m_interval = val; }
  int  GetCycleInterval() { return m_interval; }
  void Add(OnlMonClient* cli) { m_list_omc->push_back(cli); }
  void Run();

 protected:
  void BuildInterface();
  void StartAutoCycle();
  static void* FuncAutoCycle(void* arg);
  void RunAutoCycle();
};

#endif /* _ONL_MON_UI__H_ */
