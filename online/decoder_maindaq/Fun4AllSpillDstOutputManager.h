#ifndef __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__
#define __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__
#include <fun4all/Fun4AllDstOutputManager.h>
class DbSvc;

/// A Fun4All output manger that creates one DST file per spill group.
/**
 * The number of spills per group (= spill step) is 10 by default.
 * One can simply register it to Fun4AllServer as usual.
 *
 * Fun4AllSpillDstOutputManager* out_sp = new Fun4AllSpillDstOutputManager(UtilOnline::GetDstFileDir() + "/spill");
 * //out_sp->SetSpillStep(50);
 * se->registerOutputManager(out_sp);
 *
 * The status of DST outputs is recorded onto MySQL DB, once `EnableDB()` is called.
 * The table name is `spill_dst_status` by default.
 * The status is `1` when started, and `2` when finished.
 */
class Fun4AllSpillDstOutputManager: public Fun4AllDstOutputManager {
  std::string m_dir_base;
  int m_sp_step; ///< Step size to switch to next DST file, i.e. N of spills per DST.
  int m_run_id; ///< Current run ID.
  int m_sp_id_f; ///< First spill ID of current DST.  The last spill ID is `m_sp_id_f + m_sp_step - 1`.

  DbSvc* m_db;
  std::string m_name_table;

 public:
  Fun4AllSpillDstOutputManager(const std::string &dir_base, const std::string &myname = "SPILLDSTOUT");
  virtual ~Fun4AllSpillDstOutputManager();

  void SetSpillStep(const int step) { m_sp_step = step; }
  int Write(PHCompositeNode *startNode);
  void EnableDB(const bool refresh_db=false, const std::string name_table="spill_dst_status");

 protected:
  void DstStarted (const int run, const int spill_f, const int spill_l, const std::string file_name, int utime=0);
  void DstFinished(const int run, const int spill_f, const int spill_l, int utime=0);
};

#endif /* __FUN4ALL_SPILL_DST_OUTPUT_MANAGER_H__ */
