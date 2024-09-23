#ifndef __FUN4ALL_TRIGGER_DST_OUTPUT_MANAGER_H__
#define __FUN4ALL_TRIGGER_DST_OUTPUT_MANAGER_H__
#include <fun4all/Fun4AllDstOutputManager.h>

/// A Fun4All output manger that creates one DST file per run containing events selected with trigger.
class Fun4AllTriggerDstOutputManager: public Fun4AllDstOutputManager {
  int m_trig_mask; ///< Trigger bit mask to selected events to be saved.

 public:
  Fun4AllTriggerDstOutputManager(const std::string &myname, const std::string &filename);
  virtual ~Fun4AllTriggerDstOutputManager();

  void SetTriggerMask(const int fpga_mask, const int nim_mask);
  void SetTriggerMask(const bool fpga1, const bool fpga2, const bool fpga3, const bool fpga4, const bool fpga5, const bool nim1, const bool nim2, const bool nim3, const bool nim4, const bool nim5);
  virtual int Write(PHCompositeNode *startNode);
};

#endif /* __FUN4ALL_TRIGGER_DST_OUTPUT_MANAGER_H__ */
