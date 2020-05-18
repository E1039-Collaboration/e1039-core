#include "SQG4DipoleMagnetSteppingAction.h"
#include "SQG4DipoleMagnetDetector.h"
#include "PHG4StepStatusDecode.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hitv1.h>
#include <g4main/PHG4Shower.h>
#include <g4main/PHG4TrackUserInfoV1.h>

#include <phool/getClass.h>

#include <Geant4/G4Step.hh>
#include <Geant4/G4SystemOfUnits.hh>

#include <iostream>

//____________________________________________________________________________..
SQG4DipoleMagnetSteppingAction::SQG4DipoleMagnetSteppingAction(SQG4DipoleMagnetDetector* detector, const PHParameters* parameters)
  : detector_(detector)
  , params(parameters)
  , hits_(nullptr)
  , hit(nullptr)
  , saveshower(nullptr)
  , savevolpre(nullptr)
  , savevolpost(nullptr)
  , savetrackid(-1)
  , saveprestepstatus(-1)
  , savepoststepstatus(-1)
  , active(params->get_int_param("active"))
  , IsBlackHole(params->get_int_param("blackhole"))
  , use_g4_steps(params->get_int_param("use_g4steps"))
  , enable_track_filter(params->get_int_param("enable_track_filter"))
  , filter_max_slope(params->get_double_param("filter_max_slope"))
  , filter_min_energy(params->get_double_param("filter_min_energy"))
{
}

SQG4DipoleMagnetSteppingAction::~SQG4DipoleMagnetSteppingAction()
{
  // if the last hit was a zero energie deposit hit, it is just reset
  // and the memory is still allocated, so we need to delete it here
  // if the last hit was saved, hit is a nullptr pointer which are
  // legal to delete (it results in a no operation)
  delete hit;
}

//____________________________________________________________________________..
bool SQG4DipoleMagnetSteppingAction::UserSteppingAction(const G4Step* aStep, bool)
{
  if(enable_track_filter == 0) return false;

  G4TouchableHandle touch = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4TouchableHandle touchpost = aStep->GetPostStepPoint()->GetTouchableHandle();
  G4VPhysicalVolume* volume = touch->GetVolume();

  if(!detector_->IsInBlock(volume)) return false;

  //apply track filtering cuts
  G4Track* aTrack = aStep->GetTrack();

  if(aTrack->GetMomentumDirection()[2] < 0.)  //going backwards
  {
    aTrack->SetTrackStatus(fStopAndKill);
    return false;
  }

  if(aTrack->GetTotalEnergy() < 1.*MeV) // quickly kill all very soft particles
  {
    aTrack->SetTrackStatus(fStopAndKill);
    return false;
  }

  double z_pos = aTrack->GetPosition()[2]/cm;
  if(z_pos > 0. && z_pos < 500.) //in the beam dump, but energy is not sufficient to get out
  {
    double minE = (500. - z_pos)/500.*filter_min_energy*GeV;
    if(aTrack->GetTotalEnergy() < minE)
    {
      aTrack->SetTrackStatus(fStopAndKill);
      return false;
    }
  }

  return false;
}

//____________________________________________________________________________..
void SQG4DipoleMagnetSteppingAction::SetInterfacePointers(PHCompositeNode* topNode)
{
  //since magnet cannot be active, nothing is needed here
  return;
}
