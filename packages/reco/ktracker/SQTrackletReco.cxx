#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <phool/recoConsts.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include "KalmanFastTrackletting.h"
#include "EventReducer.h"
#include "SQTrackletReco.h"

SQTrackletReco::SQTrackletReco(const std::string& name)
  : SQReco(name)
  , _drop_empty_event(false)
{
  ;
}

SQTrackletReco::~SQTrackletReco()
{
  ;
}

int SQTrackletReco::process_event(PHCompositeNode* topNode) 
{
  ProcessEventPrep();

  int finderstatus = _fastfinder->setRawEvent(_rawEvent);
  if(_legacy_rec_container) 
  {
    _recEvent->setRawEvent(_rawEvent);
    _recEvent->setRecStatus(finderstatus);
  }
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) _fastfinder->printTimers();

  int nTracklets = 0;
  if(is_eval_enabled() || is_eval_dst_enabled())
  {
    for(unsigned int i = 0; i < _eval_listIDs.size(); ++i)
    {
      std::list<Tracklet>& eval_tracklets = _fastfinder->getTrackletList(_eval_listIDs[i]);
      for(auto iter = eval_tracklets.begin(); iter != eval_tracklets.end(); ++iter)
      {
        if(is_eval_enabled()) new((*_tracklets)[nTracklets]) Tracklet(*iter);
        if(is_eval_dst_enabled()) _tracklet_vector->push_back(&(*iter));
        ++nTracklets;
      }
    }
  }

  if(is_eval_enabled() && nTracklets > 0) _eval_tree->Fill();

  ProcessEventFinish();

  if (_drop_empty_event && nTracklets == 0) return Fun4AllReturnCodes::ABORTEVENT;
  return Fun4AllReturnCodes::EVENT_OK;
}

/**
 * It uses `KalmanFastTrackletting` instead of `KalmanFastTracking`.
 */
int SQTrackletReco::InitFastTracking()
{
  _fastfinder = new KalmanFastTrackletting(_phfield, _t_geo_manager, false);

  _fastfinder->Verbosity(Verbosity());

  if (_output_list_idx >= 0) _fastfinder->setOutputListIndex(_output_list_idx);
  return 0;
}

int SQTrackletReco::MakeNodes(PHCompositeNode* topNode) 
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if(!eventNode) 
  {
    LogInfo("No DST node, create one");
    eventNode = new PHCompositeNode("DST");
    topNode->addNode(eventNode);
  }

  if(_enable_eval_dst)
  {
    _tracklet_vector = findNode::getClass<TrackletVector>(topNode, "TrackletVector"); // Could exist when the tracking is re-done.
    if(!_tracklet_vector) {
      _tracklet_vector = new TrackletVector();
      _tracklet_vector->SplitLevel(99);
      eventNode->addNode(new PHIODataNode<PHObject>(_tracklet_vector, "TrackletVector", "PHObject"));
      if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/TrackletVector Added");
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}
