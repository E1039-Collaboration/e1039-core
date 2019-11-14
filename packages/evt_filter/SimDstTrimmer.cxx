#include <iomanip>
#include <fun4all/Fun4AllBase.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include "SimDstTrimmer.h"

#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl

using namespace std;

SimDstTrimmer::SimDstTrimmer(const std::string& name)
  : SubsysReco(name), _truth(0)
{
  ;
}

SimDstTrimmer::~SimDstTrimmer()
{
  ;
}

int SimDstTrimmer::Init(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SimDstTrimmer::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SimDstTrimmer::process_event(PHCompositeNode* topNode)
{
  int ret = GetNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  PHG4TruthInfoContainer::ShowerRange range = _truth->GetShowerRange();
  for (PHG4TruthInfoContainer::ShowerIterator it = range.first; it != range.second; ) {
    _truth->delete_shower(it++);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SimDstTrimmer::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SimDstTrimmer::GetNodes(PHCompositeNode* topNode)
{
  _truth = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth) {
    LogInfo("!_truth");
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

