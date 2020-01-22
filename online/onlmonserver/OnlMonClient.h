#ifndef _ONL_MON_CLIENT__H_
#define _ONL_MON_CLIENT__H_
#include <fun4all/SubsysReco.h>
#include "OnlMonCanvas.h"
class Fun4AllHistoManager;
class TSocket;
class TH1;
class TH2;
class TH3;

class OnlMonClient: public SubsysReco {
 protected:
  typedef enum { MODE_ADD, MODE_UPDATE } HistMode_t;

 private:
  std::string m_title;
  Fun4AllHistoManager* m_hm;
  int m_n_can;
  OnlMonCanvas* m_list_can[9];

  typedef std::map<std::string, HistMode_t> HistModeMap_t;
  HistModeMap_t m_hist_mode;

  typedef enum { BIN_RUN = 1, BIN_SPILL = 2, BIN_EVENT = 3, BIN_SPILL_MIN = 4, BIN_SPILL_MAX = 5 } BasicIdBin_t;
  typedef enum { BIN_N_EVT = 1, BIN_N_SP = 2 } BasicInfoBin_t;
  TH1* m_h1_basic_id;
  TH1* m_h1_basic_cnt;

  typedef std::vector<TH1*> HistList_t;
  typedef std::vector<TObject*> ObjList_t;
  HistList_t m_list_h1;

  typedef std::map<int, TH1*> SpillHistMap_t; // [spill] -> TH1*
  typedef std::map<std::string, SpillHistMap_t> Name2SpillHistMap_t; // [hist name] 
  Name2SpillHistMap_t m_map_hist_sp;
  int m_spill_id_pre;

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

  void GetBasicID(int* run_id=0, int* spill_id=0, int* event_id=0, int* spill_id_min=0, int* spill_id_max=0);
  void GetBasicCount(int* n_evt=0, int* n_sp=0);
  int StartMonitor();
  TH1* FindMonHist(const std::string name, const bool non_null=true);

  virtual int InitOnlMon(PHCompositeNode *topNode);
  virtual int InitRunOnlMon(PHCompositeNode *topNode);
  virtual int ProcessEventOnlMon(PHCompositeNode *topNode);
  virtual int EndOnlMon(PHCompositeNode *topNode);
  virtual int FindAllMonHist();
  virtual int DrawMonitor();

  static void SetClearUsFlag(const bool val) { m_bl_clear_us = val; }
  static bool GetClearUsFlag() { return m_bl_clear_us; }

  int SendHist(TSocket* sock, int sp_min, int sp_max);

 protected:  
  void RegisterHist(TH1* h1, const HistMode_t mode=MODE_ADD);

  void NumCanvases(const int num) { m_n_can = num; }
  int  NumCanvases() { return m_n_can; }
  OnlMonCanvas* GetCanvas(const int num=0);

 private:
  void MakeSpillHist();
  void MakeMergedHist(HistList_t& list_h1, const int sp_min=0, const int sp_max=0);
  int  ReceiveHist();
  void ClearHistList();
  void ClearCanvasList();
  int  DrawCanvas(const bool at_end=false);
};

#endif /* _ONL_MON_CLIENT__H_ */
