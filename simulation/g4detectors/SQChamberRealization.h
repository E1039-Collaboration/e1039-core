#ifndef SQChamberRealization_H
#define SQChamberRealization_H
#include <fun4all/SubsysReco.h>
class CalibParamXT;
class CalibParamInTimeTaiwan;
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
 * by using the X-T relation and the T0 value given by CalibParamXT and CalibParamInTimeTaiwan.
 * User has to use (i.e. register) `CalibXT` to derive the (realistic) drift distance from the TDC time,
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

private:
  double m_eff_d0;
  double m_eff_d1;
  double m_eff_d2;
  double m_eff_d3p;
  double m_eff_d3m;
  double m_eff_p1x;
  double m_eff_p1y;
  double m_eff_p2x;
  double m_eff_p2y;
  CalibParamXT* m_cal_xt;
  CalibParamInTimeTaiwan* m_cal_int;

  SQRun*       m_run;
  SQHitVector* m_hit_vec;
};

#endif
