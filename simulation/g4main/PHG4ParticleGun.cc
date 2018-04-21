#include "PHG4ParticleGun.h"
#include "PHG4Particlev1.h"

#include "PHG4InEvent.h"

#include <phool/getClass.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>

#include <Geant4/G4ParticleTable.hh>
#include <Geant4/G4ParticleDefinition.hh>

#include <TRandom.h>

#include <iostream>

using namespace std;

PHG4ParticleGun::PHG4ParticleGun(const string &name): 
  PHG4ParticleGeneratorBase(name),
	_beam_profile(nullptr)
{
  return;
}

PHG4ParticleGun::~PHG4ParticleGun()
{
	delete _beam_profile;
  return;
}

int PHG4ParticleGun::InitRun(PHCompositeNode* topNode){
	PHG4ParticleGeneratorBase::InitRun(topNode);
	gRandom->SetSeed(PHRandomSeed());
	return 0;
}

int
PHG4ParticleGun::process_event(PHCompositeNode *topNode)
{
	double vx = vtx_x, vy = vtx_y;
	if(_beam_profile) {
		_beam_profile->GetRandom2(vx, vy);
	}
	cout << "PHG4ParticleGun: {" << vx << ", " << vy << "}" << endl;

  PHG4InEvent *ineve = findNode::getClass<PHG4InEvent>(topNode,"PHG4INEVENT");
  ReuseExistingVertex(topNode); // checks if we should reuse existing vertex
  int vtxindex = ineve->AddVtx(vx,vy,vtx_z,t0);
  vector<PHG4Particle *>::const_iterator iter;
  for (iter = particlelist.begin(); iter != particlelist.end(); ++iter)
    {
      PHG4Particle *particle = new PHG4Particlev1(*iter);
      SetParticleId(particle,ineve);
      ineve->AddParticle(vtxindex, particle);
    }
  if (verbosity > 0)
    {
      ineve->identify();
    }
  return 0;
}
