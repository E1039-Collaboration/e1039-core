#include <interface_main/SQEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include "AnaWait.h"
using namespace std;

AnaWait::AnaWait(const int sec_spill, const int sec_event)
  : m_wait_spill(sec_spill), m_wait_event(sec_event)
{
  ;
}

AnaWait::~AnaWait()
{
  ;
}

int AnaWait::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaWait::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaWait::process_event(PHCompositeNode* topNode)
{
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!event) return Fun4AllReturnCodes::ABORTEVENT;

  static int id_pre = -1;
  int id = event->get_spill_id();
  if (id_pre < 0) {
    id_pre = id;
  } else if (id != id_pre) {
    id_pre = id;
    cout << "  AnaWait: " << m_wait_spill << " sec." << endl;
    DoWait(m_wait_spill);
  } else {
    DoWait(m_wait_event);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaWait::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaWait::DoWait(int sec)
{
  while (sec-- > 0) sleep(1);
}
