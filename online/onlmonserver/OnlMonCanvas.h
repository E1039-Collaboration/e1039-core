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
  //static bool m_clear_all_can;

  std::string m_name;
  std::string m_title;
  int         m_num;
  int         m_run;
  TCanvas     m_can;
  TPad        m_pad_title;
  TPad        m_pad_main;
  TPad        m_pad_msg;
  TPaveText   m_pate_msg;
  MonStatus_t m_mon_status;

 public:
  OnlMonCanvas(const std::string name, const std::string title, const int num, const int run);
  virtual ~OnlMonCanvas();

  void AddMessage(const char* msg);
  void SetStatus(const MonStatus_t stat) { m_mon_status = stat; }
  TPad* GetMainPad();

  void PreDraw();
  void PostDraw(const bool at_end=false);

  //static void WillClearAllCanvases(const bool val) { m_clear_all_can = val; }
  //static bool WillClearAllCanvases() { return m_clear_all_can; }
  //static void ClearAllCanvases();
};

#endif /* _ONL_MON_CANVAS__H_ */
