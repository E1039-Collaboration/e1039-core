/*
 * AnaMainDaq.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#ifndef _H_AnaMainDaq_H_
#define _H_AnaMainDaq_H_
#include <fun4all/SubsysReco.h>
class SQSpill;
class SQEvent;
class SQHitVector;

class AnaMainDaq: public SubsysReco {
 public:
  AnaMainDaq(const std::string &name = "AnaMainDaq");
  virtual ~AnaMainDaq() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
 private:
  void PrintSpill(SQSpill* spi);
  void PrintEvent(SQEvent* evt, SQHitVector* v_hit, SQHitVector* v_trig_hit);
};

#endif /* _H_AnaMainDaq_H_ */
