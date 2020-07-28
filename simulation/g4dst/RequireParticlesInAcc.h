#ifndef _REQUIRE_PARTICLES_IN_ACC__H_
#define _REQUIRE_PARTICLES_IN_ACC__H_
#include <map>
#include <fun4all/SubsysReco.h>
class PHG4HitContainer;

/// An SubsysReco module to select in-acceptance events.
class RequireParticlesInAcc: public SubsysReco {
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
  RequireParticlesInAcc(const std::string& name = "ACCEPTANCE");
  virtual ~RequireParticlesInAcc();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetNumHitPlanesPerParticle(const int val) { m_npl_per_par  = val; }
  void SetNumParticlesPerEvent   (const int val) { m_npar_per_evt = val; }

 private:
  int GetNodes(PHCompositeNode *topNode);
  void ExtractParticleID(const PHG4HitContainer* g4hc, std::vector<int>& vec_par_id);
  std::vector<int> ExtractParticleID(const PHG4HitContainer* g4hc_t, const PHG4HitContainer* g4hc_b);
  void CountHitPlanesPerParticle(const std::vector<int> vec_id, std::map<int, int>& map_nhit);
};

#endif /* _REQUIRE_PARTICLES_IN_ACC__H_ */
