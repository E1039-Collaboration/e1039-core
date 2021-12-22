#ifndef _FILTER_BY_TRIGGER__H_
#define _FILTER_BY_TRIGGER__H_
#include <fun4all/SubsysReco.h>
class TH1;
class SQEvent;

/// A SubsysReco module to filter events by trigger.
/**
 * It outputs the counts of all and accepted events into a tsv file once `EnableOutput()` is called.
 * 
 * Typical usage:
 * @code
 * FilterByTrigger* fbt = new FilterByTrigger();
 * fbt->SetFpgaBits(1,0,0,0,0);
 * fbt->SetNimBits (0,0,1,0,0);
 * fbt->EnableOutput();
 * se->registerSubsystem(fbt);
 * @endcode
 *
 * Advanced usage:
 * @code
 * fbt->SetTriggerBits( (0x1<<SQEvent::MATRIX1) | (0x1<<SQEvent::NIM3) );
 * @endcode
 */
class FilterByTrigger: public SubsysReco {
  unsigned short m_trig_bits;
  std::string m_fn_out;
  TH1* m_h1_evt_cnt;
  SQEvent* m_evt;

 public:
  FilterByTrigger();
  virtual ~FilterByTrigger();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetFpgaBits(const bool fpga1, const bool fpga2, const bool fpga3, const bool fpga4, const bool fpga5);
  void SetNimBits(const bool nim1, const bool nim2, const bool nim3, const bool nim4, const bool nim5);
  void SetTriggerBits(const unsigned short trig_bits) { m_trig_bits = trig_bits; }
  void EnableOutput(const std::string fn_out="filter_by_trigger.tsv") { m_fn_out = fn_out; }
};

#endif // _FILTER_BY_TRIGGER__H_
