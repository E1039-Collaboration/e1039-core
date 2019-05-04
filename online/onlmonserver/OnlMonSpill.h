#ifndef _ONL_MON_SPILL__H_
#define _ONL_MON_SPILL__H_
#include "OnlMonClient.h"
class SQSpill;

class OnlMonSpill: public OnlMonClient {
 public:
  OnlMonSpill(const std::string &name = "OnlMonSpill");
  virtual ~OnlMonSpill() {}
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  int DrawMonitor();

 private:
  void UploadToDB(SQSpill* spi);
  void PrintSpill(SQSpill* spi);
};

#endif /* _ONL_MON_SPILL__H_ */
