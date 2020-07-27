#ifndef SQCosmicGen_H__
#define SQCosmicGen_H__

#include "PHG4ParticleGeneratorBase.h"

#include <string>

#include <TRandom1.h>

class PHG4InEvent;
class PHCompositeNode;

class SQCosmicGen: public PHG4ParticleGeneratorBase 
{
public:

  SQCosmicGen(const std::string& name = "COSMICGEN");
  virtual ~SQCosmicGen() {}

  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);

  void set_mom_range(const double lo, const double hi);
  void set_theta_range(const double lo, const double hi);
  void set_charge_ratio(const double p, const double n);

  void set_acceptance_st(int stationID, bool b = true) { _req_st[stationID] = b; } 

private:
  //! the cosmic probability parameterization - p0^(cos(p1*theta))/(p2*p^2)
  double cosmicProb(double p, double theta);

  //! get a random number following uniform distribuion between lo and hi
  double uniformRand(const double lo, const double hi);

  //! calculate vtx position limits based on detector acceptance
  void getZVtxLimits(int stationID, double ty, double y, double& min, double& max);
  void getXVtxLimits(int stationID, double tx, double z, double& min, double& max);

  //! generate a set of vertex positions based on the acceptance requirement
  bool generateVtx(double tx, double ty, double& x, double& y, double& z);

  //! check acceptance in station
  inline bool acceptedInSt(int stationID, double x, double y, double z, double tx, double ty);

  //! interface to the G4 process
  PHG4InEvent* _ineve;

  //! probability of +/- charge
  double _prob_mup;
  double _prob_mum;

  //! momentum range [GeV]
  double _mom_min;
  double _mom_max;

  //! theta range
  double _theta_min;
  double _theta_max;

  //! detector acceptance cuts
  bool   _req_st[5];
  double _z_st[5];
  double _size_x_st[5];
  double _size_y_st[5];

  //! parameters for the cosmic distribution - p0^(cos(p1*theta))/(p2*p^2)
  double _p0;
  double _p1;
  double _p2;

  //! altitude of the generation plane [cm]
  double _altitude;

  //! size of the generation plane [cm]
  double _size_x;
  double _size_z;

  //! maximum probability used for normalization
  double _prob_max;

  //! Random generator
  TRandom1 _rndm;
};

#endif
