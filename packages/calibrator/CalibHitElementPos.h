#ifndef __CALIB_HIT_ELEMENT_POS_H__
#define __CALIB_HIT_ELEMENT_POS_H__
#include <fun4all/SubsysReco.h>
class SQHitVector;

/// SubsysReco module to set the position of SQHit using GeomSvc.
class CalibHitElementPos: public SubsysReco {
  bool m_cal_hit;
  bool m_cal_tr_hit;
  SQHitVector* m_vec_hit;
  SQHitVector* m_vec_trhit;

 public:
  CalibHitElementPos(const std::string &name = "CalibHitElementPos");
  virtual ~CalibHitElementPos();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void CalibHit       (const bool cal) { m_cal_hit    = cal; }
  void CalibTriggerHit(const bool cal) { m_cal_tr_hit = cal; }
};

#endif // __CALIB_HIT_ELEMENT_POS_H__
