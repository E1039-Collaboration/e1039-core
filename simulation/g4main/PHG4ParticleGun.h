#ifndef PHG4ParticleGun_H__
#define PHG4ParticleGun_H__

#include "PHG4ParticleGeneratorBase.h"

#include <TF2.h>

class PHG4Particle;

class PHG4ParticleGun: public PHG4ParticleGeneratorBase
{
 public:
  PHG4ParticleGun(const std::string &name="PGUN");
  virtual ~PHG4ParticleGun();

  virtual int InitRun(PHCompositeNode *topNode);

  int process_event(PHCompositeNode *topNode);

	TF2* get_beam_profile() const {
		return _beam_profile;
	}

	void set_beam_profile(TF2* beamProfile) {
		_beam_profile = beamProfile;
	}

 protected:

  TF2* _beam_profile;
};

#endif
