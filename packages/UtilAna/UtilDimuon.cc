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

/// Calculate the key kinematic variables of dimuon.
/**
 * Typical usage:
 * @code
 *   double mass, pT, x1, x2, xF, costh, phi;
 *   UtilDimuon::CalcVar(dim, mass, pT, x1, x2, xF, costh, phi);
 * @endcode
 *
 * The costh and phi values are of the Collins-Soper frame.
 * Please look into this function to check the formulae used.
 * The formulae were taken from SQMCDimuon::calcVariable() on 2020-11-05.
 */
void UtilDimuon::CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi)
{
  CalcVar(dim->get_mom_pos(), dim->get_mom_neg(),
          mass, pT, x1, x2, xF, costh, phi);
}

/// Calculate the key kinematic variables of dimuon.
void UtilDimuon::CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi)
{
  const Double_t mp = 0.938;
  const Double_t ebeam = 120.;
  const TLorentzVector p_beam  (0., 0., sqrt(ebeam*ebeam - mp*mp), ebeam);
  const TLorentzVector p_target(0., 0., 0., mp);
  const TLorentzVector p_cms = p_beam + p_target;

  TLorentzVector p_sum = p_pos + p_neg;
  mass = p_sum.M();
  pT   = p_sum.Perp();
  x1   = (p_target*p_sum)/(p_target*p_cms);
  x2   = (p_beam  *p_sum)/(p_beam  *p_cms);

  Double_t s     = p_cms.M2();
  Double_t sqrts = p_cms.M();
  TVector3 bv_cms = p_cms.BoostVector();
  p_sum.Boost(-bv_cms);

  xF = 2 * p_sum.Pz() / sqrts / (1 - pow(mass,2) / s);
  costh = 2 * (p_neg.E() * p_pos.Pz() - p_pos.E() * p_neg.Pz()) / mass / TMath::Sqrt(pow(mass,2) + pow(pT,2));
  phi = TMath::ATan(2*TMath::Sqrt(pow(mass,2) + pow(pT,2)) / mass * (p_neg.Px()*p_pos.Py() - p_pos.Px()*p_neg.Py()) / (p_pos.Px()*p_pos.Px() - p_neg.Px()*p_neg.Px() + p_pos.Py()*p_pos.Py() - p_neg.Py()*p_neg.Py()));
}
