#ifndef _ONL_MON_CLIENT__H_
#define _ONL_MON_CLIENT__H_
#include <TCanvas.h>
#include <TPaveText.h>
#include <fun4all/SubsysReco.h>
#include "OnlMonCanvas.h"
class TH1;
class TH2;
class TH3;
class TPaveText;

class OnlMonClient: public SubsysReco {
  std::string m_title;
  int m_n_can;
  OnlMonCanvas* m_list_can[9];

  int m_run_id;
  typedef std::vector<TH1*> HistList_t;
  HistList_t m_list_h1;
  typedef std::vector<TObject*> ObjList_t;
  ObjList_t m_list_obj;

 public:
  OnlMonClient();
  virtual ~OnlMonClient();

  void Title(const std::string &title) { m_title = title; }
  std::string Title() { return m_title; }

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);
  TObject* FindMonObj(const std::string name, const bool non_null=true);

  virtual int InitOnlMon(PHCompositeNode *topNode);
  virtual int InitRunOnlMon(PHCompositeNode *topNode);
  virtual int ProcessEventOnlMon(PHCompositeNode *topNode);
  virtual int EndOnlMon(PHCompositeNode *topNode);
  virtual int FindAllMonHist();
  virtual int DrawMonitor();

 protected:  
  int ReceiveHist();
  void ClearHistList();

  void NumCanvases(const int num) { m_n_can = num; }
  int  NumCanvases() { return m_n_can; }
  OnlMonCanvas* GetCanvas(const int num=0);
};

#endif /* _ONL_MON_CLIENT__H_ */
