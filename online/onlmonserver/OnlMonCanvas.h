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
  typedef enum { OK, WARN, ERROR, UNDEF } MonStatus_t;

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
  int         m_n_evt;

 public:
  OnlMonCanvas(const std::string name, const std::string title, const int num);
  virtual ~OnlMonCanvas();

  void SetBasicInfo(const int run_id, const int spill_id=0, const int event_id=0, const int n_evt=0);
  void AddMessage(const char* msg);
  void SetStatus(const MonStatus_t stat) { m_mon_status = stat; }
  TPad* GetMainPad();

  void  PreDraw(const bool at_end=false);
  void PostDraw(const bool at_end=false);
};

#endif /* _ONL_MON_CANVAS__H_ */
