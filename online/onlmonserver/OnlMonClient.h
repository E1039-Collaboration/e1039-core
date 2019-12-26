#ifndef _ONL_MON_CLIENT__H_
#define _ONL_MON_CLIENT__H_
#include <fun4all/SubsysReco.h>
#include "OnlMonCanvas.h"
class Fun4AllHistoManager;
class TH1;
class TH2;
class TH3;

class OnlMonClient: public SubsysReco {
  std::string m_title;
  Fun4AllHistoManager* m_hm;
  int m_n_can;
  OnlMonCanvas* m_list_can[9];

  typedef enum { BIN_RUN = 1, BIN_SPILL = 2, BIN_EVENT = 3, BIN_N_EVT = 4 } BasicInfoBin_t;
  TH1* m_h1_basic_info;
  typedef std::vector<TH1*> HistList_t;
  HistList_t m_list_h1;
  typedef std::vector<TObject*> ObjList_t;
  ObjList_t m_list_obj; ///< Need this or m_list_h1 at end...

  /// List of OnlMonClient objects created.  Used to clear all canvases opened by all objects.
  typedef std::vector<OnlMonClient*> SelfList_t;
  static SelfList_t m_list_us;
  static bool m_bl_clear_us;

 public:
  OnlMonClient();
  virtual ~OnlMonClient();
  virtual OnlMonClient* Clone();

  void Title(const std::string &title) { m_title = title; }
  std::string Title() { return m_title; }

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void GetBasicInfo(int* run_id=0, int* spill_id=0, int* event_id=0, int* n_evt=0);
  void InitCanvas();
  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);
  TObject* FindMonObj(const std::string name, const bool non_null=true);

  virtual int InitOnlMon(PHCompositeNode *topNode);
  virtual int InitRunOnlMon(PHCompositeNode *topNode);
  virtual int ProcessEventOnlMon(PHCompositeNode *topNode);
  virtual int EndOnlMon(PHCompositeNode *topNode);
  virtual int FindAllMonHist();
  virtual int DrawMonitor();

  static void SetClearUsFlag(const bool val) { m_bl_clear_us = val; }
  static bool GetClearUsFlag() { return m_bl_clear_us; }

 protected:  
  void RegisterHist(TH1* h1);

  int ReceiveHist();
  void ClearHistList();

  void NumCanvases(const int num) { m_n_can = num; }
  int  NumCanvases() { return m_n_can; }
  OnlMonCanvas* GetCanvas(const int num=0);
  void ClearCanvasList();
};

#endif /* _ONL_MON_CLIENT__H_ */
