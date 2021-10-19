#include <iomanip>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include "CalibHitElementPos.h"
using namespace std;

CalibHitElementPos::CalibHitElementPos(const std::string& name)
  : SubsysReco(name)
  , m_vec_hit   (0)
  , m_vec_trhit (0)
{
  ;
}

CalibHitElementPos::~CalibHitElementPos()
{
  ;
}

int CalibHitElementPos::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHitElementPos::InitRun(PHCompositeNode* topNode)
{
  m_vec_hit   = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  m_vec_trhit = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!m_vec_hit || !m_vec_trhit) return Fun4AllReturnCodes::ABORTEVENT;
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHitElementPos::process_event(PHCompositeNode* topNode)
{
  GeomSvc* geom = GeomSvc::instance();
  for (SQHitVector::Iter it = m_vec_hit->begin(); it != m_vec_hit->end(); it++) {
    SQHit* hit = *it;
    hit->set_pos(geom->getMeasurement(hit->get_detector_id(), hit->get_element_id()));
  }
  for (SQHitVector::Iter it = m_vec_trhit->begin(); it != m_vec_trhit->end(); it++) {
    SQHit* hit = *it;
    hit->set_pos(geom->getMeasurement(hit->get_detector_id(), hit->get_element_id()));
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHitElementPos::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
