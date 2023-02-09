#ifndef _ONL_MON_TRIG_V1495__H_
#define _ONL_MON_TRIG_V1495__H_
#include <rs_Reader/rs_Reader.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

class OnlMonTrigV1495: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:
  const char* rs_path;
  const char* rs_top_0_;
  const char* rs_top_1_;
  const char* rs_bot_0_;
  const char* rs_bot_1_;

  bool is_rs_t[2];
  bool is_rs_b[2];

  rs_Reader * rs_top[2];
  rs_Reader * rs_bot[2];

  TH1* h1_trig;
  TH2* h2_trig_time;

  TH2* h2_fpga_nim_time_b4;
  TH2* h2_fpga_nim_time_af;  

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int RF_edge_low;
  int RF_edge_up;

  TH1* h1_rs_top[2];
  TH1* h1_rs_bot[2];
  TH1* h1_rs_top_mult[2];
  TH1* h1_rs_bot_mult[2]; 
 
 public:
  OnlMonTrigV1495(const char* rs_top_0, const char* rs_top_1, const char* rs_bot_0, const char* rs_bot_1); 
  virtual ~OnlMonTrigV1495() {}
  OnlMonClient* Clone() { return new OnlMonTrigV1495(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void SetDet();
  void RoadHits(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, TH1* hist_rs, TH1* hist_mult);
  void FPGA_NIM_Time(vector<SQHit*>* FPGA, vector<SQHit*>* NIM, int NIM_trig_num, int FPGA_trig_num, TH2* hist);
};

#endif /* _ONL_MON_TRIG_V1495__H_ */
