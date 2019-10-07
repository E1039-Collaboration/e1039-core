#ifndef __CALIB_MERGE_H4_H__
#define __CALIB_MERGE_H4_H__
#include <fun4all/SubsysReco.h>
class SQHitVector;

class CalibMergeH4: public SubsysReco {
 public:
  CalibMergeH4(const std::string &name = "CalibMergeH4");
  virtual ~CalibMergeH4();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetAndMode   (const bool mode=true) { m_and_mode    = mode; }
  void SetRemoveMode(const bool mode=true) { m_remove_mode = mode; }

 private:
  bool m_and_mode;
  bool m_remove_mode;

  int MergeHits(SQHitVector* vec_in);
};

#endif /* __CALIB_MERGE_H4_H__ */
