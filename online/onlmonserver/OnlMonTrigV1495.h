#ifndef _ONL_MON_TRIG_V1495__H_
#define _ONL_MON_TRIG_V1495__H_
#define DEBUG_LVL 0
#include <rs_Reader/rs_Reader.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

class OnlMonTrigV1495: public OnlMonClient {
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

  TH2* h2_trig_time;
  TH2* h2_fpga_nim_time_af;  
  TH2* h2_RF;

  TH1* h1_trig_diff_TS;

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low[2];
  int RF_edge_up[2];
  int top;
  int bottom; 

  TH1* h1_rs_top[2];
  TH1* h1_rs_bot[2];

  vector<SQHit*>* vecH1T;
  vector<SQHit*>* vecH2T;
  vector<SQHit*>* vecH3T;
  vector<SQHit*>* vecH4T;

  vector<SQHit*>* vecH1B;
  vector<SQHit*>* vecH2B;
  vector<SQHit*>* vecH3B;
  vector<SQHit*>* vecH4B;
 
 public:
  OnlMonTrigV1495(const std::string rs_top_0, const std::string rs_top_1, const std::string rs_bot_0, const std::string rs_bot_1, const std::string rs_path=""); 
  virtual ~OnlMonTrigV1495() {}
  OnlMonClient* Clone() { return new OnlMonTrigV1495(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void debug_print(int debug_lvl);
  double Abs(double var0, double var1);
  void SetDet();
  void RoadHits(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, TH1* hist_rs,int top0_or_bot1);
  void FPGA_NIM_Time(vector<SQHit*>* FPGA, vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* h2, TH1* h1);
  void DrawTH2WithPeakPos(TH2* h2, const double cont_min=100);
};

#endif /* _ONL_MON_TRIG_V1495__H_ */
