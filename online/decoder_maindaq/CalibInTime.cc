#include <iomanip>
#include <cmath>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/CalibParamInTimeTaiwan.h>
#include <geom_svc/CalibParamInTimeV1495.h>
#include "CalibInTime.h"
using namespace std;

CalibInTime::CalibInTime(const std::string& name) : SubsysReco(name), m_cal_taiwan(0), m_cal_v1495(0)
{
  ;
}

CalibInTime::~CalibInTime()
{
  if (m_cal_taiwan) delete m_cal_taiwan;
  if (m_cal_v1495 ) delete m_cal_v1495;
}

int CalibInTime::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibInTime::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;

  if (! m_cal_taiwan) m_cal_taiwan = new CalibParamInTimeTaiwan();
  m_cal_taiwan->SetMapIDbyDB(run_header->get_run_id());
  m_cal_taiwan->ReadFromDB();

  if (! m_cal_v1495) m_cal_v1495 = new CalibParamInTimeV1495();
  m_cal_v1495->SetMapIDbyDB(run_header->get_run_id());
  m_cal_v1495->ReadFromDB();

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibInTime::process_event(PHCompositeNode* topNode)
{
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!hit_vec || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::Iter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    int ele = hit->get_element_id();
    if (det == 0) continue; /// Just skip here.  Must be warned by the channel mapper instead.

    double center, width;
    if (! m_cal_taiwan->Find(det, ele, center, width)) {
      cerr << "  WARNING:  Cannot find the in-time parameter for det=" << det << " ele=" << ele << ".\n";
      hit->set_in_time(false);
      continue;
      //return Fun4AllReturnCodes::ABORTEVENT;
    }
    hit->set_in_time( fabs(hit->get_tdc_time() - center) <= width / 2 );
  }

  for (SQHitVector::Iter it = trig_hit_vec->begin(); it != trig_hit_vec->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    int ele = hit->get_element_id();
    int lvl = hit->get_level();
    if (det == 0) continue; /// Just skip here.  Must be warned by the channel mapper instead.

    double center, width;
    if (! m_cal_v1495->Find(det, ele, lvl, center, width)) {
      cerr << "  WARNING:  Cannot find the in-time parameter for trigger det=" << det << " ele=" << ele << " lvl=" << lvl << ".\n";
      hit->set_in_time(false);
      continue;
      //return Fun4AllReturnCodes::ABORTEVENT;
    }
    hit->set_in_time( fabs(hit->get_tdc_time() - center) <= width / 2 );
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibInTime::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
