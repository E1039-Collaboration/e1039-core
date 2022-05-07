#ifndef _SQ_TRACKLET_RECO_H
#define _SQ_TRACKLET_RECO_H
#include "SQReco.h"

class SQTrackletReco: public SQReco
{
 public:
  bool _drop_empty_event;

  SQTrackletReco(const std::string& name = "SQTrackletReco");
  virtual ~SQTrackletReco();

  virtual int process_event(PHCompositeNode* topNode);

  virtual void drop_empty_event(const bool val) { _drop_empty_event = val; }

 private:
  virtual int InitFastTracking();
  virtual int MakeNodes(PHCompositeNode* topNode);
};

#endif // _SQ_TRACKLET_RECO_H
