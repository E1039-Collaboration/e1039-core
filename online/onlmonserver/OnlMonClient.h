#ifndef _ONL_MON_CLIENT__H_
#define _ONL_MON_CLIENT__H_
#include <TCanvas.h>
#include <fun4all/SubsysReco.h>
class TH1;
class TH2;
class TH3;

class OnlMonClient: public SubsysReco {
 public:
  OnlMonClient(const std::string &name = "OnlMonClient");
  virtual ~OnlMonClient();

  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);

  virtual int DrawMonitor();

 protected:  
  TCanvas* c1;
  TPad* pad_title;
  TPad* pad_main ;
  TPad* pad_msg  ;

  int ReceiveHist();

 private:
  typedef std::vector<TH1*> HistList_t;
  HistList_t m_list_h1;
  void ClearHistList();
};

#endif /* _ONL_MON_CLIENT__H_ */
