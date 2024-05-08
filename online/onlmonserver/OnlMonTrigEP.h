#ifndef _ONL_MON_TRIG_EP__H_
#define _ONL_MON_TRIG_EP__H_
#define DEBUG_LVL 0
#include <rs_Reader/rs_Reader.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

/// OnlMonClient to estimate the efficiency and the purity of FPGA trigger.
class OnlMonTrigEP: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  std::string rs_top_0_;
  std::string rs_top_1_;
  std::string rs_bot_0_;
  std::string rs_bot_1_;
  std::string rs_path_; ///< Path to folder containing all of the roadset .txt files

  bool is_rs_t[2];
  bool is_rs_b[2];

  rs_Reader * rs_top[2];
  rs_Reader * rs_bot[2];

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];
  int top;
  int bottom;

  TH1* h1_purity;  
  TH1* h1_eff_NIM4;

  int rs_top_check_p[2];
  int rs_bot_check_p[2];  

  int rs_top_check_e[2];
  int rs_bot_check_e[2];
  
  vector<SQHit*>* vecH1T;
  vector<SQHit*>* vecH2T;
  vector<SQHit*>* vecH3T;
  vector<SQHit*>* vecH4T;

  vector<SQHit*>* vecH1B;
  vector<SQHit*>* vecH2B;
  vector<SQHit*>* vecH3B;
  vector<SQHit*>* vecH4B;
 
 public:
  OnlMonTrigEP(const std::string rs_top_0, const std::string rs_top_1, const std::string rs_bot_0, const std::string rs_bot_1, const std::string rs_path="");
  virtual ~OnlMonTrigEP() {}
  OnlMonClient* Clone() { return new OnlMonTrigEP(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void debug_print(int debug_lvl);
  void SetDet();
  int RoadCheck(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj,int top0_or_bot1);
};

#endif /* _ONL_MON_TRIG_EP__H_ */
