#ifndef _E906_SPILL_SELECTOR__H_
#define _E906_SPILL_SELECTOR__H_
#include <vector>
#include <fun4all/SubsysReco.h>
class TH1;
class SQEvent;

/// A SubsysReco module to select good E906 spills.
/**
 * It can handle only the last E906 dataset, Run 6.
 *
 * It outputs the counts of all and accepted events into a tsv file once `EnableOutput()` is called.
 * 
 * Typical usage:
 * @code
 * E906SpillSelector* e906ss = new E906SpillSelector();
 * e906ss->EnableOutput();
 * se->registerSubsystem(e906ss);
 * @endcode
 */
class E906SpillSelector: public SubsysReco {
  std::vector<int> m_list_spill_ok;
  std::string m_fn_list;
  std::string m_fn_out;
  TH1* m_h1_evt_cnt;
  SQEvent* m_evt;

 public:
  E906SpillSelector(const std::string &name="E906SpillSelector");
  virtual ~E906SpillSelector();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetSpillListFile(const std::string fn_list) { m_fn_list = fn_list; }
  void EnableOutput(const std::string fn_out="e906_spill_selector.tsv") { m_fn_out = fn_out; }
};

#endif // _E906_SPILL_SELECTOR__H_
