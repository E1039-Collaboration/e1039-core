#ifndef _DB_UP_SPILL__H_
#define _DB_UP_SPILL__H_
#include <fun4all/SubsysReco.h>
class SQSpill;

class DbUpSpill: public SubsysReco {
 public:
  DbUpSpill(const std::string &name = "DbUpSpill");
  virtual ~DbUpSpill() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 private:
  void UploadToDB(SQSpill* spi);
  void PrintSpill(SQSpill* spi);
};

#endif /* _DB_UP_SPILL__H_ */
