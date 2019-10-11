#include <iomanip>
//#include <TH1D.h>
#include <TF1.h>
#include <interface_main/SQHit.h>
#include <geom_svc/GeomSvc.h>
#include "UtilHodo.h"
using namespace std;
namespace UtilHodo {

bool IsHodoX(const std::string det_name)
{
  return det_name.length() >= 3 && (det_name[2] == 'T' || det_name[2] == 'B');
}

bool IsHodoY(const std::string det_name)
{
  return ! IsHodoX(det_name);
}

bool IsHodoX(const int det_id)
{
  return IsHodoX( GeomSvc::instance()->getDetectorName(det_id) );
}

bool IsHodoY(const int det_id)
{
  return ! IsHodoX(det_id);
}

int GetPlanePos(const int det, double& x, double& y, double& z)
{
  GeomSvc* geom = GeomSvc::instance();
  x = geom->getPlaneCenterX(det);
  y = geom->getPlaneCenterY(det);
  z = geom->getPlaneCenterZ(det);
  return 0;
}

int GetElementPos(const int det, const int ele, double& x, double& y, double& z)
{
  GeomSvc* geom = GeomSvc::instance();
  GetPlanePos(det, x, y, z);
  int n_ele = geom->getPlaneNElements(det);
  double pos_ele = (ele - (n_ele+1)/0.5) * geom->getPlaneSpacing(det);
  if (IsHodoX(det)) x += pos_ele;
  else              y += pos_ele;
  return 0;
}

////////////////////////////////////////////////////////////////
Track1D::Track1D(const XY_t xy) : type_xy(xy), chi2(0), pos(0), slope(0), ndf(0)
{
  ;
}

int Track1D::DoTracking()
{
  if (list_hit.size() < 2) return 1;

  for (unsigned int ih = 0; ih < list_hit.size(); ih++) {
    short det = list_hit[ih]->get_detector_id();
    short ele = list_hit[ih]->get_element_id();
    double x_ele, y_ele, z_ele;
    GetElementPos(det, ele, x_ele, y_ele, z_ele);
    if (type_xy == X) graph.SetPoint(ih, x_ele, z_ele);
    else              graph.SetPoint(ih, y_ele, z_ele);
  }
  graph.Fit("pol1", "Q0");
  TF1* f1 = graph.GetFunction("pol1");
  chi2  = f1->GetChisquare();
  ndf   = f1->GetNDF();
  pos   = f1->GetParameter(0);
  slope = f1->GetParameter(1);
  return 0;
}

////////////////////////////////////////////////////////////////
Track2D::Track2D() : trk_x(Track1D::X), trk_y(Track1D::Y)
{
  ;
}

void Track2D::AddHit(SQHit* hit)
{
  if (IsHodoX(hit->get_detector_id())) trk_x.list_hit.push_back(hit);
  else                                 trk_y.list_hit.push_back(hit);
}

int Track2D::DoTracking()
{
  if (trk_x.DoTracking() != 0) return 1;
  if (trk_y.DoTracking() != 0) return 2;
  return 0;
}

int Track2D::GetNDF()
{
  return trk_x.ndf + trk_y.ndf;
}

double Track2D::GetChi2()
{
  return trk_x.chi2 + trk_y.chi2;
}

TVector3 Track2D::GetPos0()
{
  return TVector3(trk_x.pos, trk_y.pos, 0);
}

TVector3 Track2D::GetSlope()
{
  return TVector3(trk_x.slope, trk_y.slope, 1);
}

TVector3 Track2D::GetPos(const double z)
{
  TVector3 vec(GetPos0());
  vec += z * GetSlope();
  return vec;
}

////////////////////////////////////////////////////////////////
void HodoTrack::AddHit(SQHit* hit)
{
  if (IsHodoX(hit->get_detector_id())) m_list_hit_x.push_back(hit);
  else                                 m_list_hit_y.push_back(hit);
}

int HodoTrack::DoTracking()
{
  m_chi2 = m_ndf = 0;
  m_pos0 .SetXYZ(0, 0, 0);
  m_slope.SetXYZ(0, 0, 0);
  if (m_list_hit_x.size() < 2) return 1;
  if (m_list_hit_y.size() < 2) return 2;

  DoTracking1D(true , &m_list_hit_x, &m_gr_x);
  DoTracking1D(false, &m_list_hit_y, &m_gr_y);
  TF1* f1_x = m_gr_x.GetFunction("pol1");
  TF1* f1_y = m_gr_y.GetFunction("pol1");
  m_chi2 = f1_x->GetChisquare() + f1_y->GetChisquare();
  m_ndf  = f1_x->GetNDF()       + f1_y->GetNDF();
  m_pos0 .SetXYZ( f1_x->GetParameter(0), f1_y->GetParameter(0), 0.0 );
  m_slope.SetXYZ( f1_x->GetParameter(1), f1_y->GetParameter(1), 1.0 );

  return 0;
}

int HodoTrack::DoTracking1D(const bool is_x, HitList_t* list_hit, TGraph* gr)
{
  for (unsigned int ih = 0; ih < list_hit->size(); ih++) {
    short det = list_hit->at(ih)->get_detector_id();
    short ele = list_hit->at(ih)->get_element_id();
    double x_ele, y_ele, z_ele;
    GetElementPos(det, ele, x_ele, y_ele, z_ele);
    if (is_x) gr->SetPoint(ih, x_ele, z_ele);
    else      gr->SetPoint(ih, y_ele, z_ele);
  }
  gr->Fit("pol1", "Q0");
  return 0;
}

TVector3 HodoTrack::GetPos(const double z)
{
  TVector3 vec(m_pos0);
  vec += z*m_slope;
  return vec;
}

};
