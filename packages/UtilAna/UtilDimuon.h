#ifndef _UTIL_DIMUON__H_
#define _UTIL_DIMUON__H_
class SQDimuon;
class SQDimuonVector;
class TVector3;
class TLorentzVector;

namespace UtilDimuon {
  SQDimuon* FindDimuonByID(const SQDimuonVector* vec, const int id_dim, const bool do_assert=false);

  void GetX1X2(const SQDimuon* dim, double& x1, double& x2);
  void GetX1X2(const SQDimuon* dim,  float& x1,  float& x2);
  double GetX1(const SQDimuon* dim);
  double GetX2(const SQDimuon* dim);

  void CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF);
  void CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF);

  void CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi);
  void CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi);

  void Lab2CollinsSoper(const SQDimuon* dim, double& costh, double& phi);
  void Lab2CollinsSoper(const TLorentzVector& p1, const TLorentzVector& p2, double& costh, double& phi);
  void Lab2CollinsSoper(const TVector3& p1, const TVector3& p2, double& costh, double& phi);
  void Lab2CollinsSoper(const double px1, const double py1, const double pz1,
                        const double px2, const double py2, const double pz2, double& costh, double& phi);
}

#endif /* _UTIL_DIMUON__H_ */
