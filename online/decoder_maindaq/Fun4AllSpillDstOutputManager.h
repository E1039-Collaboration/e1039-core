#ifndef __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__
#define __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__
#include <fun4all/Fun4AllDstOutputManager.h>

/// A Fun4All output manger that creates one DST file per spill group.
/**
 * The number of spills per group (= spill step) is 10 by default.
 * One can simply register it to Fun4AllServer as usual.
 *
 * Fun4AllSpillDstOutputManager* out_sp = new Fun4AllSpillDstOutputManager(UtilOnline::GetDstFileDir() + "/spill");
 * //out_sp->SetSpillStep(50);
 * se->registerOutputManager(out_sp);
 */
class Fun4AllSpillDstOutputManager: public Fun4AllDstOutputManager {
  std::string m_dir_base;
  int m_sp_step;
  int m_run_id;
  int m_sp_id;

 public:
  Fun4AllSpillDstOutputManager(const std::string &dir_base, const std::string &myname = "SPILLDSTOUT");
  virtual ~Fun4AllSpillDstOutputManager();

  void SetSpillStep(const int step) { m_sp_step = step; }
  int Write(PHCompositeNode *startNode);
};

#endif /* __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__ */
