#include <iomanip>
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

TGraph* Track2D::GetGraphX()
{
  return &trk_x.graph;
}

TGraph* Track2D::GetGraphY()
{
  return &trk_y.graph;
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

}; // End of "namespace UtilHodo"
