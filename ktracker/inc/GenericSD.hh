#ifndef GenericSD_h
#define GenericSD_h 1

#include "G4VSensitiveDetector.hh"
#include "MCHit.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4ios.hh"

class G4Step;
class G4HCofThisEvent;

class GenericSD : public G4VSensitiveDetector
{
  public:
    GenericSD(G4String);
    ~GenericSD();

    void Initialize(G4HCofThisEvent*);
    G4bool ProcessHits(G4Step*, G4TouchableHistory*);
    void EndOfEvent(G4HCofThisEvent*);

  private:
    MCHitsCollection* staHitsCollection;

};

#endif
