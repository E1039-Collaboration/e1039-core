#ifndef _DB_UP_SPILL__H_
#define _DB_UP_SPILL__H_
#include <fun4all/SubsysReco.h>
class SQSpill;
class SQHardSpill;

class DbUpSpill: public SubsysReco {
 public:
  DbUpSpill(const std::string &name = "DbUpSpill");
  virtual ~DbUpSpill() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 private:
  void ClearTable(const char* table_name, const int run_id);
  void UploadToSpillTable(SQSpill* spi, SQHardSpill* hspi);
  void UploadToScalerTable(SQSpill* spi, const std::string boseos);
  void UploadToSlowContTable(SQSpill* spi);
  void PrintSpill(SQSpill* spi, SQHardSpill* hspi);
};

#endif /* _DB_UP_SPILL__H_ */
