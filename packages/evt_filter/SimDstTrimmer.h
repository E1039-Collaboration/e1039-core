#ifndef SimDstTrimmer_H
#define SimDstTrimmer_H
//#include <vector>
//#include <string>
//#include <iostream>
//#include <set>
//#include <list>
//#include <map>
#include <fun4all/SubsysReco.h>
class PHG4TruthInfoContainer;

class SimDstTrimmer : public SubsysReco {
 public:
  SimDstTrimmer(const std::string &name = "SimDstTrimmer");
  virtual ~SimDstTrimmer();

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);
    
 private:
  PHG4TruthInfoContainer* _truth;

  int GetNodes(PHCompositeNode *topNode);
};


#endif
