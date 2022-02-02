#ifndef SQChamberRealization_H
#define SQChamberRealization_H
#include <GlobalConsts.h>
#include <fun4all/SubsysReco.h>
class CalibParamXT;
class SQRun;
class SQHitVector;

/// SubsysReco module to do the detector realization for chamber and prop tube.
/**
 * This module simulates the detection efficiency and the time/distance resolution.
 * 
 * The in-time flag of one SQHit object is set to 'false' when the hit is regarded as inefficient.
 * We might better have another flag like 'is_efficient' in future.
 *
 * The drift distance of one SQHit object is converted to its TDC time,
 * by using the X-T relation given by CalibParamXT.
 * User has to use (i.e. register) `CalibDriftDist` to derive the (realistic) drift distance from the TDC time,
 * because this module does _not_ modify the drift distance itself.
 */
class SQChamberRealization: public SubsysReco
{
public:
  SQChamberRealization(const std::string& name = "SQChamberRealization");
  virtual ~SQChamberRealization();

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void SetChamEff(const double eff_d0, const double eff_d1, const double eff_d2, const double eff_d3p, const double eff_d3m);
  void SetPropTubeEff(const double eff_p1x, const double eff_p1y, const double eff_p2x, const double eff_p2y);

  void ScaleChamReso(const double scale_d0, const double scale_d1, const double scale_d2, const double scale_d3p, const double scale_d3m);

  void FixChamReso(const double reso_d0, const double reso_d1, const double reso_d2, const double reso_d3p, const double reso_d3m);

private:
  struct PlaneParam {
    bool   on;
    double eff;
    double reso_scale;
    double reso_fixed;
    PlaneParam() : on(false), eff(1.0), reso_scale(1.0), reso_fixed(-1) {;}
  };
  PlaneParam list_param[nChamberPlanes+nHodoPlanes+nPropPlanes+1];

  CalibParamXT* m_cal_xt;

  SQRun*       m_run;
  SQHitVector* m_hit_vec;
};

#endif
