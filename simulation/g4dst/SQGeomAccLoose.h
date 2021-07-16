#ifndef _SQ_GEOM_ACC_LOOSE__H_
#define _SQ_GEOM_ACC_LOOSE__H_
#include <map>
#include <fun4all/SubsysReco.h>
class PHG4HitContainer;

/// An SubsysReco module to skip a simulated event in which a muon or a muon pair doesn't pass through the _rough_ geometric acceptance.
/**
 * Typical usage:
 * @code
 *   SQGeomAccLoose* geom_acc = new SQGeomAccLoose();
 *   geom_acc->SetNumParticlesPerEvent(2); // or 1 for single events
 *   se->registerSubsystem(geom_acc);
 * @endcode
 *
 * This module can/should be registered before `SQDigitizer` for better process speed.
 * The geometric acceptance required by this module is _rough_, because 
 *     - The chamber acceptance is not included and
 *     - The hodoscope plane area is defined by the Geant4 volume and thus larger than its real size.
 * You have to use `SQGeomAcc` when you need accurate and/or flexible geometric acceptance.
 */
class SQGeomAccLoose: public SubsysReco {
  int m_npl_per_par; //< Min number of hit planes per particle in order to regard particle good.
  int m_npar_per_evt; //< Min number of particles per event in order to regard event good.

  PHG4HitContainer *g4hc_h1t;
  PHG4HitContainer *g4hc_h1b;
  PHG4HitContainer *g4hc_h2t;
  PHG4HitContainer *g4hc_h2b;
  PHG4HitContainer *g4hc_h3t;
  PHG4HitContainer *g4hc_h3b;
  PHG4HitContainer *g4hc_h4t;
  PHG4HitContainer *g4hc_h4b;

 public:
  SQGeomAccLoose(const std::string& name = "SQGeomAccLoose");
  virtual ~SQGeomAccLoose();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetNumHitPlanesPerParticle(const int val) { m_npl_per_par  = val; }
  void SetNumParticlesPerEvent   (const int val) { m_npar_per_evt = val; }

 private:
  void ExtractParticleID(const PHG4HitContainer* g4hc, std::vector<int>& vec_par_id);
  std::vector<int> ExtractParticleID(const PHG4HitContainer* g4hc_t, const PHG4HitContainer* g4hc_b);
  void CountHitPlanesPerParticle(const std::vector<int> vec_id, std::map<int, int>& map_nhit);
};

#endif /* _SQ_GEOM_ACC_LOOSE__H_ */
