#include <iomanip>
#include <TGraphErrors.h>
#include <interface_main/SQParamDeco.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/recoConsts.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <geom_svc/CalibParamXT.h>
#include "CalibDriftDist.h"
using namespace std;

CalibDriftDist::CalibDriftDist(const std::string& name)
  : SubsysReco(name)
  , m_manual_map_selection(false)
  , m_fn_xt("")
  , m_vec_hit(0)
  , m_cal_xt (0)
{
  ;
}

CalibDriftDist::~CalibDriftDist()
{
  if (m_cal_xt ) delete m_cal_xt ;
}

int CalibDriftDist::Init(PHCompositeNode* topNode)
{
  if (m_manual_map_selection) {
    m_cal_xt = new CalibParamXT();
    m_cal_xt->SetMapID(m_fn_xt);
    m_cal_xt->ReadFromLocalFile(m_fn_xt);
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibDriftDist::InitRun(PHCompositeNode* topNode)
{
  //SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  //if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  m_vec_hit = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!m_vec_hit) return Fun4AllReturnCodes::ABORTEVENT;

  recoConsts* rc = recoConsts::instance();

  if (! m_manual_map_selection) {
    int run_id = rc->get_IntFlag("RUNNUMBER");
    m_cal_xt = new CalibParamXT();
    m_cal_xt->SetMapIDbyDB(run_id); // (run_header->get_run_id());
    m_cal_xt->ReadFromDB();
  }

  SQParamDeco* param_deco = findNode::getClass<SQParamDeco>(topNode, "SQParamDeco");
  if (param_deco) {
    param_deco->set_variable(m_cal_xt->GetParamID(), m_cal_xt->GetMapID());
  }

  rc->set_CharFlag(m_cal_xt->GetParamID(), m_cal_xt->GetMapID());
  if (Verbosity() > 0) {
    cout << Name() << ": " << m_cal_xt->GetParamID() << " = " << m_cal_xt->GetMapID() << "\n";
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibDriftDist::process_event(PHCompositeNode* topNode)
{
  GeomSvc* geom = GeomSvc::instance();
  for (SQHitVector::Iter it = m_vec_hit->begin(); it != m_vec_hit->end(); it++) {
    SQHit* hit = *it;
    int det = hit->get_detector_id();
    if (!geom->isChamber(det) && !geom->isPropTube(det)) continue;

    CalibParamXT::Set* xt = m_cal_xt->GetParam(det);
    if (! det) {
      cerr << "  WARNING:  Cannot find the in-time parameter for det=" << det << " in CalibDriftDist.\n";
      continue;
      //return Fun4AllReturnCodes::ABORTEVENT;
    }
    float   tdc_time = hit->get_tdc_time();
    float drift_dist = xt->t2x.Eval(tdc_time);
    if (drift_dist < 0) drift_dist = 0;
    hit->set_drift_distance(drift_dist);
    hit->set_in_time(xt->T1 <= tdc_time && tdc_time <= xt->T0);
    /// No field for resolution in SQHit now.

    int ele = hit->get_element_id();
    hit->set_pos(geom->getMeasurement(det, ele));
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibDriftDist::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void CalibDriftDist::ReadParamFromFile(const char* fn_xt_curve)
{
  m_manual_map_selection = true;
  m_fn_xt  = fn_xt_curve;
}
