#ifndef _UTIL_DIMUON__H_
#define _UTIL_DIMUON__H_
class SQDimuon;
class TLorentzVector;

namespace UtilDimuon {
  void GetX1X2(const SQDimuon* dim, double& x1, double& x2);
  void GetX1X2(const SQDimuon* dim,  float& x1,  float& x2);
  double GetX1(const SQDimuon* dim);
  double GetX2(const SQDimuon* dim);

  void CalcVar(const SQDimuon* dim, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi);
  void CalcVar(const TLorentzVector& p_pos, const TLorentzVector& p_neg, double& mass, double& pT, double& x1, double& x2, double& xF, double& costh, double& phi);
}

#endif /* _UTIL_DIMUON__H_ */
