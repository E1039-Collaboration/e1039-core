#ifndef __FUN4ALLSRAWEVENTOUTPUTMANAGER_H__
#define __FUN4ALLSRAWEVENTOUTPUTMANAGER_H__
#include <fun4all/Fun4AllOutputManager.h>
#include <string>
#include <vector>
class TFile;
class TTree;
class PHCompositeNode;
class SRawEvent;
class SQEvent;
class SQSpillMap;
class SQHitVector;

class Fun4AllSRawEventOutputManager: public Fun4AllOutputManager
{
  typedef enum {
    UNDEF  = 0,
    OPEN   = 1,
    UPDATE = 2,
    CLOSE  = 3
  } Status_t;
  std::string m_dir_base;
  std::string m_tree_name;
  std::string m_branch_name;
  std::string m_file_name;
  int m_run_id;
  int m_spill_id;
  TFile* m_file;
  TTree* m_tree;
  SRawEvent* m_sraw;
  
  SQEvent* m_evt;
  SQSpillMap* m_sp_map;
  SQHitVector* m_hit_vec;
  SQHitVector* m_trig_hit_vec;

 public:
  Fun4AllSRawEventOutputManager(const std::string &dir_base, const std::string &myname = "SRAWEVENTOUT");
  virtual ~Fun4AllSRawEventOutputManager();

  void SetTreeName  (const std::string name) { m_tree_name   = name; }
  void SetBranchName(const std::string name) { m_branch_name = name; }

  virtual int Write(PHCompositeNode *startNode);

 protected:
  void CloseFile();
  void OpenFile();
  void UpdateDBStatus(const int status);
};

#endif /* __FUN4ALLSRAWEVENTOUTPUTMANAGER_H__ */
