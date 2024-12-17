#ifndef SQSingleMuonGen_H__
#define SQSingleMuonGen_H__

#include <g4main/PHG4ParticleGeneratorBase.h>
#include <string>

#include <TVector3.h>
#include <TRandom2.h>
#include <TF1.h>

#include "SQSingleMuonTruthInfo.h"

class PHG4InEvent;
class PHG4Particle;
class PHG4VtxPoint;
class PHCompositeNode;

class SQSingleMuonGen: public PHG4ParticleGeneratorBase 
{
public:

  SQSingleMuonGen(const std::string& name = "SINGLEGEN");
  virtual ~SQSingleMuonGen();

  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);

  const void set_target_only(bool en = true)   { _target_only = en; }
  const void enable_geom_cut(bool en = true)   { _enable_geom_cut = en; }
  const void set_mother_decay_length(double l) { _ctau = l; }
  const void set_charge_ratio(double v)        { _charge_ratio = v; }
  const void set_pt_max(double v)              { _pt_max = v; }
  const void set_mom_range(double min, double max)  { _mom_min = min; _mom_max = max; }
  const void enable_real_mom_dist(bool en = true)   { _enable_real_mom_dist = en; }

private:
  //! generate a set of vertex positions based on the acceptance requirement
  bool generatePrimaryVtx();

  //! Function to generate and decay a pion
  void generateMotherParticle();
  void decayMotherParticle();

  //! Check if the muon satisfy the geometry cuts
  bool geometryCut();

  //! interface to the G4 process
  PHG4InEvent* _ineve;

  //! Container of the mother particle to be saved on DST
  SQSingleMuonTruthInfo* _truth;

  //! Minimum and maximum mother particle momentum
  double _mom_min;
  double _mom_max;
  double _pt_max;

  //! target only flag
  bool _target_only;

  //! enable geometry cut
  bool _enable_geom_cut;

  //! enable realistic pion momentum distribution
  bool _enable_real_mom_dist;

  //! Mother particle decay length
  double _ctau;

  //! pi+/pi- ratio
  double _charge_ratio;

  //! Random generator
  TRandom2 _rndm;

  //! Hardcoded mass for muon and pion
  const double _m_muon;
  const double _m_pion;

  //! Distribution functions for p/pt distribution
  TF1* _pz_dist[6];
  TF1* _pt_dist;
};

#endif
