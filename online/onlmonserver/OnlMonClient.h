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
  int m_n_can;
  OnlMonCanvas* m_list_can[9];

  typedef std::vector<TH1*> HistList_t;
  HistList_t m_list_h1;
  typedef std::vector<TObject*> ObjList_t;
  ObjList_t m_list_obj;

 public:
  OnlMonClient(const std::string &name = "OnlMonClient");
  virtual ~OnlMonClient();

  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);
  TObject* FindMonObj(const std::string name, const bool non_null=true);

  virtual int DrawMonitor();

 protected:  
  int ReceiveHist();
  void ClearHistList();

  void SetNumCanvases(const int num) { m_n_can = num; }
  OnlMonCanvas* GetCanvas(const int num=0);
};

#endif /* _ONL_MON_CLIENT__H_ */
