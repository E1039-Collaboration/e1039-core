#ifndef _ONL_MON_CANVAS__H_
#define _ONL_MON_CANVAS__H_
#include <TCanvas.h>
#include <TPaveText.h>
#include <fun4all/SubsysReco.h>
class TH1;
class TH2;
class TH3;
class TPaveText;

class OnlMonCanvas {
 public:
  typedef enum { OK = 0, WARN = 1, ERROR = 2, UNDEF = 3 } MonStatus_t;

 protected:
  std::string m_name;
  std::string m_title;
  int         m_num;
  TCanvas     m_can;
  TPad        m_pad_title;
  TPad        m_pad_main;
  TPad        m_pad_msg;
  TPaveText   m_pate_msg;
  MonStatus_t m_mon_status;

  int         m_run;
  int         m_spill;
  int         m_event;
  int         m_spill_min;
  int         m_spill_max;
  int         m_n_evt;
  int         m_n_sp;

 public:
  OnlMonCanvas(const std::string name, const std::string title, const int num);
  virtual ~OnlMonCanvas();

  void SetBasicID(const int run_id, const int spill_id=0, const int event_id=0, const int spill_id_min=0, const int spill_id_max=0);
  void SetBasicCount(const int n_evt=0, const int n_sp=0);
  void AddMessage(const char* msg);
  void AddMessage(const std::string msg) { AddMessage(msg.c_str()); }
  MonStatus_t GetStatus() { return m_mon_status; }
  void SetStatus(const MonStatus_t stat) { m_mon_status = stat; }
  void SetWorseStatus(const MonStatus_t stat);
  TPad* GetMainPad();

  void  PreDraw(const bool at_end=false);
  void PostDraw(const bool at_end=false);
};

#endif /* _ONL_MON_CANVAS__H_ */
