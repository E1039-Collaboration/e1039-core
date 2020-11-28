#include "PHG4PhenixSteppingAction.h"
#include "PHG4SteppingAction.h"

#include <Geant4/G4Track.hh>
#include <Geant4/G4SystemOfUnits.hh>

PHG4PhenixSteppingAction::PHG4PhenixSteppingAction(): energy_threshold_(-1.)
{}

PHG4PhenixSteppingAction::~PHG4PhenixSteppingAction()
{
  while (actions_.begin() != actions_.end())
    {
      delete actions_.back();
      actions_.pop_back();
    }
}


//_________________________________________________________________
void PHG4PhenixSteppingAction::UserSteppingAction( const G4Step* aStep )
{
  // kill tracks to save time
  if(energy_threshold_ > 0.)
  {
    G4Track* theTrack = aStep->GetTrack();
    if(theTrack->GetMomentumDirection()[2] < 0.) //track is going backwards
    {
      theTrack->SetTrackStatus(fStopAndKill);
      return;
    }

    if(theTrack->GetTotalEnergy() < energy_threshold_*GeV)
    {
      theTrack->SetTrackStatus(fStopAndKill);
      return;
    }
  }

  // loop over registered actions, and process
  bool hit_was_used = false;
  for( ActionList::const_iterator iter = actions_.begin(); iter != actions_.end(); ++iter )
  {
    if(*iter)
    {
      hit_was_used |= (*iter)->UserSteppingAction( aStep, hit_was_used );
    }
  }

}
