#include <TRandom.h>
#include <TGraphErrors.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <phool/recoConsts.h>
#include <geom_svc/GeomSvc.h>
#include <geom_svc/CalibParamXT.h>
#include <geom_svc/CalibParamInTimeTaiwan.h>
#include "SQChamberRealization.h"
using namespace std;

SQChamberRealization::SQChamberRealization(const std::string& name)
  : SubsysReco(name)
  , m_eff_d0 (1.)
  , m_eff_d1 (1.)
  , m_eff_d2 (1.)
  , m_eff_d3p(1.)
  , m_eff_d3m(1.)
  , m_eff_p1x(1.)
  , m_eff_p1y(1.)
  , m_eff_p2x(1.)
  , m_eff_p2y(1.)
  , m_cal_xt (0)
  , m_cal_int(0)
  , m_run    (0)
  , m_hit_vec(0)
{
  ;
}

SQChamberRealization::~SQChamberRealization() 
{
  if (m_cal_xt ) delete m_cal_xt ;
  if (m_cal_int) delete m_cal_int;
}

int SQChamberRealization::Init(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::InitRun(PHCompositeNode* topNode) 
{
  PHNodeIterator iter(topNode);
  m_run = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!m_run) {
    cerr << Name() << "SQRun node missing.  Abort." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  m_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!m_hit_vec) {
    cerr << Name() << "SQHitVector node missing.  Abort." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  m_cal_xt = new CalibParamXT();
  m_cal_xt->SetMapIDbyDB(m_run->get_run_id());
  m_cal_xt->ReadFromDB();
  recoConsts::instance()->set_CharFlag("CHAM_REAL_XT", m_cal_xt->GetMapID());

  m_cal_int = new CalibParamInTimeTaiwan();
  m_cal_int->SetMapIDbyDB(m_run->get_run_id());
  m_cal_int->ReadFromDB();
  recoConsts::instance()->set_CharFlag("CHAM_REAL_INTIME", m_cal_int->GetMapID());

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::process_event(PHCompositeNode* topNode) 
{
  for (SQHitVector::Iter it = m_hit_vec->begin(); it != m_hit_vec->end(); it++) {
    SQHit* hit = *it;
    int    det_id   = hit->get_detector_id();
    string det_name = GeomSvc::instance()->getDetectorName(det_id);

    bool is_eff = true;
    if      (det_name.substr(0, 2) == "D0" ) is_eff = (gRandom->Rndm() < m_eff_d0 );
    else if (det_name.substr(0, 2) == "D1" ) is_eff = (gRandom->Rndm() < m_eff_d1 );
    else if (det_name.substr(0, 2) == "D2" ) is_eff = (gRandom->Rndm() < m_eff_d2 );
    else if (det_name.substr(0, 3) == "D3p") is_eff = (gRandom->Rndm() < m_eff_d3p);
    else if (det_name.substr(0, 3) == "P1X") is_eff = (gRandom->Rndm() < m_eff_p1x);
    else if (det_name.substr(0, 3) == "P1Y") is_eff = (gRandom->Rndm() < m_eff_p1y);
    else if (det_name.substr(0, 3) == "P2X") is_eff = (gRandom->Rndm() < m_eff_p2x);
    else if (det_name.substr(0, 3) == "P2Y") is_eff = (gRandom->Rndm() < m_eff_p2y);
    hit->set_in_time(is_eff); // Temporary solution!!

    TGraphErrors* gr_x2t;
    TGraphErrors* gr_x2dt;
    if (m_cal_xt->FindX2T(det_id, gr_x2t, gr_x2dt)) {
      int    ele_id  = hit->get_element_id();
      double dist    = hit->get_drift_distance();
      double mean_t  = gr_x2t ->Eval(dist);
      double width_t = gr_x2dt->Eval(dist);
      double drift_time = -1;
      while (drift_time < 0) drift_time = gRandom->Gaus(mean_t, width_t);

      double t0 = 0;
      double center_int, width_int;
      if (m_cal_int->Find(det_id, ele_id, center_int, width_int)) t0 = center_int + width_int / 2;
      float tdc_time = t0 - drift_time;

      hit->set_tdc_time(tdc_time);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::End(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void SQChamberRealization::SetChamEff(const double eff_d0, const double eff_d1, const double eff_d2, const double eff_d3p, const double eff_d3m)
{
  m_eff_d0  = eff_d0;
  m_eff_d1  = eff_d1;
  m_eff_d2  = eff_d2;
  m_eff_d3p = eff_d3p;
  m_eff_d3m = eff_d3m;
}

void SQChamberRealization::SetPropTubeEff(const double eff_p1x, const double eff_p1y, const double eff_p2x, const double eff_p2y)
{
  m_eff_p1x = eff_p1x;
  m_eff_p1y = eff_p1y;
  m_eff_p2x = eff_p2x;
  m_eff_p2y = eff_p2y;
}
