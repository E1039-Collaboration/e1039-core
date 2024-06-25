#include <iomanip>
#include <interface_main/SQParamDeco.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/recoConsts.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <geom_svc/CalibParamInTimeTaiwan.h>
#include <geom_svc/CalibParamInTimeV1495.h>
#include "CalibHodoInTime.h"
using namespace std;

CalibHodoInTime::CalibHodoInTime(const std::string& name)
  : SubsysReco(name)
  , m_vec_hit   (0)
  , m_vec_trhit (0)
  , m_cal_taiwan(0)
  , m_cal_v1495 (0)
{
  ;
}

CalibHodoInTime::~CalibHodoInTime()
{
  if (m_cal_taiwan) delete m_cal_taiwan;
  if (m_cal_v1495 ) delete m_cal_v1495;
}

int CalibHodoInTime::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHodoInTime::InitRun(PHCompositeNode* topNode)
{
  //SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  //if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  m_vec_hit   = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  m_vec_trhit = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!m_vec_hit || !m_vec_trhit) return Fun4AllReturnCodes::ABORTEVENT;

  recoConsts* rc = recoConsts::instance();
  int run_id = rc->get_IntFlag("RUNNUMBER");

  m_cal_taiwan = new CalibParamInTimeTaiwan();
  m_cal_taiwan->SetMapIDbyDB(run_id); // (run_header->get_run_id());
  m_cal_taiwan->ReadFromDB();

  m_cal_v1495 = new CalibParamInTimeV1495();
  m_cal_v1495->SetMapIDbyDB(run_id); // (run_header->get_run_id());
  m_cal_v1495->ReadFromDB();

  SQParamDeco* param_deco = findNode::getClass<SQParamDeco>(topNode, "SQParamDeco");
  if (param_deco)  {
    param_deco->set_variable(m_cal_taiwan->GetParamID(), m_cal_taiwan->GetMapID());
    param_deco->set_variable(m_cal_v1495 ->GetParamID(), m_cal_v1495 ->GetMapID());
  }

  rc->set_CharFlag(m_cal_taiwan->GetParamID(), m_cal_taiwan->GetMapID());
  rc->set_CharFlag(m_cal_v1495 ->GetParamID(), m_cal_v1495 ->GetMapID());

  if (Verbosity() > 0) {
    cout << Name() << ": " << m_cal_taiwan->GetParamID() << " = " << m_cal_taiwan->GetMapID() << "\n"
         << Name() << ": " << m_cal_v1495 ->GetParamID() << " = " << m_cal_v1495 ->GetMapID() << "\n";
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHodoInTime::process_event(PHCompositeNode* topNode)
{
  GeomSvc* geom = GeomSvc::instance();
  for (SQHitVector::Iter it = m_vec_hit->begin(); it != m_vec_hit->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    if (!geom->isHodo(det) && !geom->isDPHodo(det) && !geom->isInh(det)) continue;

    int ele = hit->get_element_id();
    double center, width;
    if (! m_cal_taiwan->Find(det, ele, center, width)) {
      if (Verbosity() > 1) cerr << "  WARNING:  Cannot find the in-time parameter for det=" << det << " ele=" << ele << ".\n";
      //hit->set_in_time(false);
      continue;
    }
    hit->set_in_time( fabs(hit->get_tdc_time() - center) <= width / 2 );
    hit->set_drift_distance(0);
  }

  for (SQHitVector::Iter it = m_vec_trhit->begin(); it != m_vec_trhit->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    if (!geom->isHodo(det)) continue;

    int ele = hit->get_element_id();
    int lvl = hit->get_level();
    double center, width;
    if (! m_cal_v1495->Find(det, ele, lvl, center, width)) {
      if (Verbosity() > 1) cerr << "  WARNING:  Cannot find the in-time parameter for trigger det=" << det << " ele=" << ele << " lvl=" << lvl << ".\n";
      //hit->set_in_time(false);
      continue;
    }
    hit->set_in_time( fabs(hit->get_tdc_time() - center) <= width / 2 );
    hit->set_drift_distance(0);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibHodoInTime::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
