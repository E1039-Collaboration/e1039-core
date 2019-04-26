#ifndef _ONL_MON_CLIENT__H_
#define _ONL_MON_CLIENT__H_
#include <TCanvas.h>
#include <fun4all/SubsysReco.h>
class TH1;
class TH2;
class TH3;
class TPaveText;

class OnlMonClient: public SubsysReco {
 protected:  
  typedef enum { OK, WARN, ERROR, UNDEF } MonStatus_t;
  TCanvas* c1;
  TPad* pad_title;
  TPad* pad_main ;
  TPad* pad_msg  ;

 private:
  typedef std::vector<TH1*> HistList_t;
  HistList_t m_list_h1;
  typedef std::vector<TObject*> ObjList_t;
  ObjList_t m_list_obj;
  void ClearHistList();

  /// Variables for message and status
  MonStatus_t mon_status;
  TPaveText* pate_msg;

 public:
  OnlMonClient(const std::string &name = "OnlMonClient");
  virtual ~OnlMonClient();

  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);
  TObject* FindMonObj(const std::string name, const bool non_null=true);

  virtual int DrawMonitor();

 protected:  
  int ReceiveHist();

  /// Functions for message and status
  void AddMessage(const char* msg);
  void SetStatus(const MonStatus_t stat) { mon_status = stat; }
};

#endif /* _ONL_MON_CLIENT__H_ */
