#include "EvtFilter.h"


#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <fun4all/Fun4AllBase.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl

using namespace std;

int EvtFilter::Init(PHCompositeNode *topNode)
{
 return Fun4AllReturnCodes::EVENT_OK;
}

EvtFilter::EvtFilter(const std::string& name) :
  SubsysReco(name),
  _event(0),
  _trigger_req(0),
  _event_id_req(-1),
  _event_header(nullptr)
{
}

EvtFilter::~EvtFilter()
{
}

int EvtFilter::InitRun(PHCompositeNode* topNode) {

  return Fun4AllReturnCodes::EVENT_OK;
}

int EvtFilter::process_event(PHCompositeNode* topNode) {

  if(Verbosity() >= Fun4AllBase::VERBOSITY_EVEN_MORE)
    std::cout << "Entering EvtFilter::process_event: " << _event << std::endl;

  int ret = GetNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK)
    return ret;

  if(_event_header) {
    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
      _event_header->identify();
    }

    unsigned short trig = _event_header->get_trigger();

    if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT)
      LogInfo("_trigger_req: " << _trigger_req << ", trig" << trig);

    if(_trigger_req!=0) {
      if((_trigger_req & trig) == 0) {
        if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("_trigger_req & trig == 0");
        return Fun4AllReturnCodes::ABORTEVENT;
      }
    }

    if(_event_id_req > -1) {

      if(_event_id_req != _event_header->get_event_id()) {
        if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) LogInfo("_trigger_req & trig == 0");
        return Fun4AllReturnCodes::ABORTEVENT;
      }
    }
  }
    
  if(Verbosity() >= Fun4AllBase::VERBOSITY_EVEN_MORE)
    std::cout << "Leaving EvtFilter::process_event: " << _event++ << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;

}

int EvtFilter::End(PHCompositeNode* topNode) {
  return Fun4AllReturnCodes::EVENT_OK;
}

int EvtFilter::GetNodes(PHCompositeNode* topNode) {
  
  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogInfo("!_event_header");
    return Fun4AllReturnCodes::ABORTRUN;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

