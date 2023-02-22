#ifndef _ONL_MON_HODO_V1495__H_
#define _ONL_MON_HODO_V1495__H_
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

class OnlMonHodoV1495: public OnlMonClient {
 public:
  typedef enum { H1X, H2X, H3X, H4X, H1Y, H2Y, H4Y1, H4Y2 } HodoType_t;
  static const int N_DET = 8;

 private:

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int event_cnt;

  TH1* h1_ele     [N_DET];
  TH1* h1_ele_in  [N_DET];
  TH1* h1_time    [N_DET];
  TH1* h1_time_in [N_DET];

  //TH2* h2_time_ele_raw[N_DET];
  TH2* h2_time_ele_fpga[N_DET];
 
  TH1* RF_proj[8];
  TLine* proj_line[8];
  TLine* proj_line_H1[8];
  
  TH2* h2_RF;
  TH2* h2_RF_raw;
 public:
  OnlMonHodoV1495();
  virtual ~OnlMonHodoV1495() {}
  OnlMonClient* Clone() { return new OnlMonHodoV1495(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void SetDet();
};

#endif /* _ONL_MON_HODO_V1495__H_ */
