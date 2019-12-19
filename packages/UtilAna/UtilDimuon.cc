#include <TLorentzVector.h>
#include <interface_main/SQDimuon.h>
#include "UtilDimuon.h"
using namespace std;

void UtilDimuon::GetX1X2(const SQDimuon* dim, double& x1, double& x2)
{
  const double M_P    = 0.938;
  const double E_BEAM = 120.0;
  const TLorentzVector p_beam  (0, 0, sqrt(E_BEAM*E_BEAM - M_P*M_P), E_BEAM);
  const TLorentzVector p_target(0, 0, 0, M_P);
  const TLorentzVector p_cms = p_beam + p_target;
  TLorentzVector p_sum = dim->get_mom_pos() + dim->get_mom_neg();
  x1 = (p_target*p_sum)/(p_target*p_cms);
  x2 = (p_beam  *p_sum)/(p_beam  *p_cms);
}

void UtilDimuon::GetX1X2(const SQDimuon* dim, float& x1, float& x2)
{
  double x1d, x2d;
  GetX1X2(dim, x1d, x2d);
  x1 = x1d;
  x2 = x2d;
}

double UtilDimuon::GetX1(const SQDimuon* dim)
{
  double x1, x2;
  GetX1X2(dim, x1, x2);
  return x1;
}

double UtilDimuon::GetX2(const SQDimuon* dim)
{
  double x1, x2;
  GetX1X2(dim, x1, x2);
  return x2;
}
