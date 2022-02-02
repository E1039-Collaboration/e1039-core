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
#include "SQChamberRealization.h"
using namespace std;

SQChamberRealization::SQChamberRealization(const std::string& name)
  : SubsysReco(name)
  , m_cal_xt (0)
  , m_run    (0)
  , m_hit_vec(0)
{
  ;
}

SQChamberRealization::~SQChamberRealization() 
{
  if (m_cal_xt ) delete m_cal_xt ;
}

int SQChamberRealization::Init(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::InitRun(PHCompositeNode* topNode) 
{
  //PHNodeIterator iter(topNode);
  //m_run = findNode::getClass<SQRun>(topNode, "SQRun");
  //if (!m_run) {
  //  cerr << Name() << ": SQRun node missing.  Abort." << endl;
  //  return Fun4AllReturnCodes::ABORTRUN;
  //}
  m_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!m_hit_vec) {
    cerr << Name() << ": SQHitVector node missing.  Abort." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  recoConsts* rc = recoConsts::instance();
  int run_id = rc->get_IntFlag("RUNNUMBER");

  m_cal_xt = new CalibParamXT();
  m_cal_xt->SetMapIDbyDB(run_id); // (m_run->get_run_id());
  m_cal_xt->ReadFromDB();
  rc->set_CharFlag("CHAM_REAL_XT", m_cal_xt->GetMapID());

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::process_event(PHCompositeNode* topNode) 
{
  if (Verbosity() >= 2) cout << "SQChamberRealization:  n_hits=" << m_hit_vec->size() << endl;
  SQHitVector::Iter it = m_hit_vec->begin();
  while (it != m_hit_vec->end()) {
    SQHit* hit = *it;
    int det_id = hit->get_detector_id();
    PlaneParam* param = &list_param[det_id];
    if (! param->on) {
      it++;
      continue;
    }

    int  ele_id = hit->get_element_id();
    bool is_eff = (gRandom->Rndm() < param->eff);
    if (Verbosity() >= 2) {
      string det_name = GeomSvc::instance()->getDetectorName(det_id);
      cout << "  Hit: det=" << det_id << ":" << det_name << " ele=" << ele_id << " is_eff=" << is_eff << endl;
    }
    if (! is_eff) {
      it = m_hit_vec->erase(it);
      continue;
    }

    CalibParamXT::Set* xt = m_cal_xt->GetParam(det_id);
    if (xt) {
      double dist = hit->get_drift_distance();
      int dist_sign = dist > 0  ?  +1  :  -1;
      dist *= dist_sign; // Made positive.

      double dx;
      if (param->reso_fixed >= 0) dx  = param->reso_fixed;
      else                        dx  = xt->x2dx.Eval(dist);
      if (param->reso_scale >= 0) dx *= param->reso_scale;
      if (Verbosity() >= 2) cout << "       dist=" << dist << " dx=" << dx << " X0=" << xt->X0 << " X1=" << xt->X1 << endl;

      double dist_new;
      int n_try = 10000;
      while (n_try > 0) {
        dist_new = gRandom->Gaus(dist, dx);
        if (xt->X0 <= dist_new && dist_new <= xt->X1) break;
        n_try--;
      }
      if (n_try == 0) {
        cout << PHWHERE << " Failed at generating an in-range drift time.  Something unexpected.  Abort." << endl;
        exit(1);
      }
      hit->set_drift_distance(dist_new * dist_sign);
      hit->set_tdc_time(xt->x2t.Eval(dist_new));
    }
    it++;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQChamberRealization::End(PHCompositeNode *topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void SQChamberRealization::SetChamEff(const double eff_d0, const double eff_d1, const double eff_d2, const double eff_d3p, const double eff_d3m)
{
  GeomSvc* geom = GeomSvc::instance();
  if (Verbosity() > 0) cout << "SQChamberRealization::SetChamEff():\n";
  for (int ii = 1; ii <= nChamberPlanes; ii++) {
    string name = geom->getDetectorName(ii);
    double eff = -1;
    if      (name.substr(0, 2) == "D0" ) eff = eff_d0;
    else if (name.substr(0, 2) == "D1" ) eff = eff_d1;
    else if (name.substr(0, 2) == "D2" ) eff = eff_d2;
    else if (name.substr(0, 3) == "D3p") eff = eff_d3p;
    else if (name.substr(0, 3) == "D3m") eff = eff_d3m;
    if (eff >= 0) {
      list_param[ii].on  = true;
      list_param[ii].eff = eff;
      if (Verbosity() > 0) cout << "  " << setw(2) << ii << ":" << setw(5) << name << " = " << eff << endl;
    }
  }
}

void SQChamberRealization::SetPropTubeEff(const double eff_p1x, const double eff_p1y, const double eff_p2x, const double eff_p2y)
{
  GeomSvc* geom = GeomSvc::instance();
  if (Verbosity() > 0) cout << "SQChamberRealization::SetProTubeEff():\n";
  for (int ii = nChamberPlanes+nHodoPlanes + 1; ii <= nChamberPlanes+nHodoPlanes+nPropPlanes; ii++) {
    string name = geom->getDetectorName(ii);
    double eff = -1;
    if      (name.substr(0, 3) == "P1X") eff = eff_p1x;
    else if (name.substr(0, 3) == "P1Y") eff = eff_p1y;
    else if (name.substr(0, 3) == "P2X") eff = eff_p2x;
    else if (name.substr(0, 3) == "P2Y") eff = eff_p2y;
    if (eff >= 0) {
      list_param[ii].on  = true;
      list_param[ii].eff = eff;
      if (Verbosity() > 0) cout << "  " << setw(2) << ii << ":" << setw(5) << name << " = " << eff << endl;
    }
  }
}

void SQChamberRealization::ScaleChamReso(const double scale_d0, const double scale_d1, const double scale_d2, const double scale_d3p, const double scale_d3m)
{
  GeomSvc* geom = GeomSvc::instance();
  if (Verbosity() > 0) cout << "SQChamberRealization::ScaleChamReso():\n";
  for (int ii = 1; ii <= nChamberPlanes; ii++) {
    string name = geom->getDetectorName(ii);
    double scale = -1;
    if      (name.substr(0, 2) == "D0" ) scale = scale_d0 ;
    else if (name.substr(0, 2) == "D1" ) scale = scale_d1 ;
    else if (name.substr(0, 2) == "D2" ) scale = scale_d2 ;
    else if (name.substr(0, 3) == "D3p") scale = scale_d3p;
    else if (name.substr(0, 3) == "D3m") scale = scale_d3m;
    if (scale >= 0) {
      list_param[ii].on         = true;
      list_param[ii].reso_scale = scale;
      if (Verbosity() > 0) cout << "  " << setw(2) << ii << ":" << setw(5) << name << " = " << scale << endl;
    }
  }
}

void SQChamberRealization::FixChamReso(const double reso_d0, const double reso_d1, const double reso_d2, const double reso_d3p, const double reso_d3m)
{
  GeomSvc* geom = GeomSvc::instance();
  if (Verbosity() > 0) cout << "SQChamberRealization::FixChamReso():\n";
  for (int ii = 1; ii <= nChamberPlanes; ii++) {
    string name = geom->getDetectorName(ii);
    double reso = -1;
    if      (name.substr(0, 2) == "D0" ) reso = reso_d0 ;
    else if (name.substr(0, 2) == "D1" ) reso = reso_d1 ;
    else if (name.substr(0, 2) == "D2" ) reso = reso_d2 ;
    else if (name.substr(0, 3) == "D3p") reso = reso_d3p;
    else if (name.substr(0, 3) == "D3m") reso = reso_d3m;
    if (reso >= 0) {
      list_param[ii].on         = true;
      list_param[ii].reso_fixed = reso;
      if (Verbosity() > 0) cout << "  " << setw(2) << ii << ":" << setw(5) << name << " = " << reso << endl;
    }
  }
}
