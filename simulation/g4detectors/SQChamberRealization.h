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
 * When it regards a hit as inefficient, it erases the hit from SQHitVector.
 * The efficiency should be set via `SetChamEff()` and `SetPropTubeEff()`.
 *
 * It smears the drift distance of a hit using dx of the X-T curve by default.
 * The resolution (`dx`) can be changed via `ScaleChamReso()` and `FixChamReso()`.
 * It sets the TDC time based on the smeared drift distance and the X-T curve.
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

  /// Set the single-plane efficiency of the chamber planes.
  void SetChamEff(const double eff_d0, const double eff_d1, const double eff_d2, const double eff_d3p, const double eff_d3m);

  /// Set the single-plane efficiency of the prop-tube planes.
  void SetPropTubeEff(const double eff_p1x, const double eff_p1y, const double eff_p2x, const double eff_p2y);

  /// Set the scaling factor of the single-plane resolution of the chamber planes.
  void ScaleChamReso(const double scale_d0, const double scale_d1, const double scale_d2, const double scale_d3p, const double scale_d3m);

  /// Set the single-plane resolution of the chamber planes to the given values.
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
