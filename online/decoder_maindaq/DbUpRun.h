#ifndef _DB_UP_RUN__H_
#define _DB_UP_RUN__H_
#include <fun4all/SubsysReco.h>
class SQRun;
class SQParamDeco;

class DbUpRun: public SubsysReco {
 public:
  DbUpRun(const std::string &name = "DbUpRun");
  virtual ~DbUpRun() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 private:
  void UploadRun(SQRun* sq);
  void UploadParam(const int run, const SQParamDeco* sq);
  void UploadV1495(SQRun* sq);
};

#endif /* _DB_UP_RUN__H_ */
