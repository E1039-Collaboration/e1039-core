#ifndef _ONL_MON_H4__H_
#define _ONL_MON_H4__H_
#include "OnlMonClient.h"

class OnlMonH4: public OnlMonClient {
 public:
  typedef enum { H4T, H4B, H4Y1L, H4Y1R, H4Y2L, H4Y2R } HodoType_t;

 private:
  static const int N_PL = 2;
  HodoType_t m_type;
  std::string m_det_name;
  std::vector<short> m_list_det;
  std::vector<std::string> m_list_det_name;

  TH1* h1_ele [N_PL];
  TH1* h1_time[N_PL];
  TH1* h2_ele ;
  TH1* h2_time;

 public:
  OnlMonH4(const HodoType_t type);
  virtual ~OnlMonH4() {}
  OnlMonClient* Clone() { return new OnlMonH4(*this); }

  int InitOnlMon(PHCompositeNode *topNode);
  int InitRunOnlMon(PHCompositeNode *topNode);
  int ProcessEventOnlMon(PHCompositeNode *topNode);
  int EndOnlMon(PHCompositeNode *topNode);
  int FindAllMonHist();
  int DrawMonitor();
};

#endif /* _ONL_MON_H4__H_ */
