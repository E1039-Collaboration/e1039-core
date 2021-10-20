#include <cassert>
#include <TLorentzVector.h>
#include <interface_main/SQDimuonVector.h>
#include "UtilDimuon.h"
using namespace std;

/// Find a dimuon by dimuon ID in the given dimuon list.
/**
 * This function returns a SQDimuon pointer if successful.
 * Otherwise it returns '0' by default, or aborts when 'do_assert' = true.
 */
SQDimuon* UtilDimuon::FindDimuonByID(const SQDimuonVector* vec, const int id_dim, const bool do_assert)
{
  for (SQDimuonVector::ConstIter it = vec->begin(); it != vec->end(); it++) {
    SQDimuon* dim = *it;
    if (dim->get_dimuon_id() == id_dim) {
      if (do_assert) assert(dim);
      return dim;
    }
  }
  return 0;
}

/// OBSOLETE: Use `CalcVar()` instead.
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

/// OBSOLETE: Use `CalcVar()` instead.
void UtilDimuon::GetX1X2(const SQDimuon* dim, float& x1, float& x2)
{
  double x1d, x2d;
  GetX1X2(dim, x1d, x2d);
  x1 = x1d;
  x2 = x2d;
}

/// OBSOLETE: Use `CalcVar()` instead.
double UtilDimuon::GetX1(const SQDimuon* dim)
{
  double x1, x2;
  GetX1X2(dim, x1, x2);
  return x1;
}

/// OBSOLETE: Use `CalcVar()` instead.
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
 *   double mass, pT, x1, x2, xF;
 *   UtilDimuon::CalcVar(dim, mass, pT, x1, x2, xF);
 * @endcode
 */
void UtilDimuon::CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF)
{
  CalcVar(dim->get_mom_pos(), dim->get_mom_neg(), mass, pT, x1, x2, xF);
}

/// Calculate the key kinematic variables of dimuon.
/**
 * Typical usage:
 * @code
 *   double mass, pT, x1, x2, xF;
 *   UtilDimuon::CalcVar(mom_pos, mom_neg, mass, pT, x1, x2, xF);
 * @endcode
 */
void UtilDimuon::CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF)
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
}

/// OBSOLETE:  Use `CalcVar(dim, mass, pT, x1, x2, xF)` and `Lab2CollinsSoper(dim, costh, phi)` instead.
/**
 * Typical usage:
 * @code
 *   double mass, pT, x1, x2, xF, costh, phi;
 *   UtilDimuon::CalcVar(dim, mass, pT, x1, x2, xF, costh, phi);
 * @endcode
 */
void UtilDimuon::CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi)
{
  CalcVar(dim->get_mom_pos(), dim->get_mom_neg(), mass, pT, x1, x2, xF, costh, phi);
}

/// OBSOLETE:  Use `CalcVar(dim, mass, pT, x1, x2, xF)` and `Lab2CollinsSoper(dim, costh, phi)` instead.
void UtilDimuon::CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi)
{
  CalcVar(p_pos, p_neg, mass, pT, x1, x2, xF);
  Lab2CollinsSoper(p_pos.Vect(), p_neg.Vect(), costh, phi);
}

/// Convert the momenta of muon pair from Lab frame to Collins-Soper frame.
void UtilDimuon::Lab2CollinsSoper(const SQDimuon* dim, double& costh, double& phi)
{
  Lab2CollinsSoper(dim->get_mom_pos().Vect(), dim->get_mom_neg().Vect(), costh, phi);
}

/// Convert the momenta of muon pair from Lab frame to Collins-Soper frame.
void UtilDimuon::Lab2CollinsSoper(const TLorentzVector& p1, const TLorentzVector& p2, double& costh, double& phi)
{
  Lab2CollinsSoper(p1.Vect(), p2.Vect(), costh, phi);
}

/// Convert the momenta of muon pair from Lab frame to Collins-Soper frame.
void UtilDimuon::Lab2CollinsSoper(const TVector3& p1, const TVector3& p2, double& costh, double& phi)
{
  Lab2CollinsSoper(p1.X(), p1.Y(), p1.Z(),  p2.X(), p2.Y(), p2.Z(),  costh, phi);
}

/// Convert the momenta of muon pair from Lab frame to Collins-Soper frame.
/** 
 * The code was written originally by Suguru Tamamushi.
 * Only the cos(theta) and phi are returned at present.
 */
void UtilDimuon::Lab2CollinsSoper(const double px1, const double py1, const double pz1,
                      const double px2, const double py2, const double pz2, double& costh, double& phi)
{
  const double m_mu = 0.1056;
  TLorentzVector mom1;
  TLorentzVector mom2;
  mom1.SetXYZM(px1, py1, pz1, m_mu); //Momentum of muon 1 at Laboratory Frame
  mom2.SetXYZM(px2, py2, pz2, m_mu); //Momentum of muon 2 at Laboratory Frame
  
  //Lorentz Boost to Hadron Rest Frame
  const double m_p  = 0.938;
  const double E_p  = 120.0;
  const double p_p  = sqrt(E_p*E_p - m_p*m_p);
  const double beta = p_p/E_p;
  mom1.Boost(0,0,-beta);  //mom1.Pz is now boosted
  mom2.Boost(0,0,-beta);  //mom2.Pz is now boosted

  //Calculate costheta
  TLorentzVector Q = mom1 + mom2;
  double k1p = mom1.E() + mom1.Pz();
  double k2p = mom2.E() + mom2.Pz();
  double k1m = mom1.E() - mom1.Pz();
  double k2m = mom2.E() - mom2.Pz();
  costh = 1.0/Q.M()/sqrt(Q*Q + Q.Px()*Q.Px() + Q.Py()*Q.Py())*(k1p*k2m - k1m*k2p);

  //Calculate tanphi
  TVector3 Delta(mom1.Px() - mom2.Px(), mom1.Py() - mom2.Py(), mom1.Pz() - mom2.Pz());
  TVector3 Q3(Q.Px(),Q.Py(),Q.Pz());
  TVector3 PA(0, 0, p_p);
  TVector3 R = PA.Cross(Q3);
  TVector3 RHat = R.Unit();
  TVector3 QT = Q3;
  QT.SetZ(0);
  double tanphi = sqrt(Q*Q + Q.Px()*Q.Px() + Q.Py()*Q.Py()) / Q.M() * (Delta.X()*RHat.X() + Delta.Y()*RHat.Y()) / (Delta.X()*QT.Unit().X() + Delta.Y()*QT.Unit().Y());

  //Calculate Phi Quadrant
  double sinth = sin( acos(costh) );
  double sinphi = (Delta.X()*RHat.X() + Delta.Y()*RHat.Y())/(Q.M()*sinth);

  //Calculate Phi
  phi = atan(tanphi);
  if      (tanphi >= 0 && sinphi >= 0) {;} // phi  = phi;}
  else if (tanphi <  0 && sinphi >  0) {phi +=   TMath::Pi();}
  else if (tanphi >  0 && sinphi <  0) {phi +=   TMath::Pi();}
  else if (tanphi <  0 && sinphi <  0) {phi += 2*TMath::Pi();}
}

