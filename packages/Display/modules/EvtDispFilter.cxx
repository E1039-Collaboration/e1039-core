#include <sstream>
//#include <TSystem.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "EvtDispFilter.h"
using namespace std;

EvtDispFilter::EvtDispFilter(const int n_step, const int n_max)
  : SubsysReco("EvtDispFilter")
  , m_n_step(n_step)
  , m_n_max(n_max)
  , m_sq_evt(0)
  , m_sq_hv(0)
{
  ;
}

EvtDispFilter::~EvtDispFilter()
{
  ;
}

int EvtDispFilter::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int EvtDispFilter::InitRun(PHCompositeNode* topNode)
{
  m_sq_evt = findNode::getClass<SQEvent    >(topNode, "SQEvent"    );
  m_sq_hv  = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!m_sq_evt || !m_sq_hv) return Fun4AllReturnCodes::ABORTEVENT;
  return Fun4AllReturnCodes::EVENT_OK;
}

int EvtDispFilter::process_event(PHCompositeNode* topNode)
{
  static int spill_id_pre = -1;
  static int event_id_pre = -1;
  static int n_evt_sp = 0;

  int spill_id = m_sq_evt->get_spill_id();
  if (spill_id_pre < 0 || spill_id != spill_id_pre) { // 1st or new spill
    n_evt_sp = 0;
    spill_id_pre = spill_id;
  }
  if (n_evt_sp >= m_n_max) return Fun4AllReturnCodes::DISCARDEVENT;

  int event_id = m_sq_evt->get_event_id();
  if (event_id - event_id_pre < m_n_step) return Fun4AllReturnCodes::DISCARDEVENT;

  if (m_sq_evt->get_trigger(SQEvent::NIM1   ) ||
      m_sq_evt->get_trigger(SQEvent::NIM2   ) ||
//      m_sq_evt->get_trigger(SQEvent::MATRIX1) ||
      m_sq_evt->get_trigger(SQEvent::NIM4   )   ) {
    event_id_pre = event_id;
    n_evt_sp++;
    return Fun4AllReturnCodes::EVENT_OK;
  }
  return Fun4AllReturnCodes::DISCARDEVENT;
}

int EvtDispFilter::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
