#ifndef _ONL_MON_TRIG_RS__H_
#define _ONL_MON_TRIG_RS__H_
#include <rs_Reader/rs_Reader.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "OnlMonClient.h"

class OnlMonTrigRS: public OnlMonClient {
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

  

  HodoType_t m_type;
  int m_lvl;
  std::string list_det_name[N_DET];
  int         list_det_id  [N_DET];

  int event_cnt;

  TH1* h1_rs_top[2];
  TH1* h1_rs_bot[2];
  TH1* h1_rs_top_mult[2];
  TH1* h1_rs_bot_mult[2]; 
 
 public:
  OnlMonTrigRS(const char* rs_top_0, const char* rs_top_1, const char* rs_bot_0, const char* rs_bot_1); 
  virtual ~OnlMonTrigRS() {}
  OnlMonClient* Clone() { return new OnlMonTrigRS(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();

 private:
  void SetDet();
  void RoadHits(vector<SQHit*>* H1X, vector<SQHit*>* H2X, vector<SQHit*>* H3X, vector<SQHit*>* H4X,rs_Reader* rs_obj, TH1* hist_rs, TH1* hist_mult);
};

#endif /* _ONL_MON_TRIG_RS__H_ */
