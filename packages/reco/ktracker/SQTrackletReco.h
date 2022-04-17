#ifndef _SQ_TRACKLET_RECO_H
#define _SQ_TRACKLET_RECO_H
#include "SQReco.h"

class SQTrackletReco: public SQReco
{
 public:
  SQTrackletReco(const std::string& name = "SQTrackletReco");
  virtual ~SQTrackletReco();

  virtual int process_event(PHCompositeNode* topNode);

 private:
  virtual int InitFastTracking();
  virtual int MakeNodes(PHCompositeNode* topNode);
};

#endif // _SQ_TRACKLET_RECO_H
