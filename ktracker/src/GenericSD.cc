// Based on ExN02TrackerSD.hh from ParN02 example

#include "GenericSD.hh"

GenericSD::GenericSD(G4String name): G4VSensitiveDetector(name)
{
  G4String HCname;
  collectionName.insert(HCname="staHitsCollection");
}

GenericSD::~GenericSD()
{
}

void GenericSD::Initialize(G4HCofThisEvent* HCE)
{
  staHitsCollection = new MCHitsCollection(SensitiveDetectorName, collectionName[0]); 
  static G4int HCID = -1;
  if(HCID<0)
    HCID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  HCE->AddHitsCollection(HCID, staHitsCollection);
}

G4bool GenericSD::ProcessHits(G4Step* aStep,G4TouchableHistory*)
{
  return true;

  G4double edep = aStep->GetTotalEnergyDeposit();

  G4ParticleDefinition* particle = aStep->GetTrack()->GetDefinition();
  if (particle->GetPDGCharge() == 0.)
    return false;

  G4Track* theTrack = aStep->GetTrack();

  MCHit* newHit = new MCHit();
  newHit->SetParticleID(particle->GetPDGEncoding());
  newHit->SetParticleName(particle->GetParticleName());
  newHit->SetPosition(aStep->GetTrack()->GetPosition());
  newHit->SetMomentum(theTrack->GetMomentum());
  newHit->SetVertex(theTrack->GetVertexPosition());
  newHit->SetTime(theTrack->GetGlobalTime());   
  newHit->SetTrackID(theTrack->GetTrackID());
  newHit->SetParentID(theTrack->GetParentID());
  newHit->SetVertexMomentumDirection(theTrack->GetVertexMomentumDirection());
  newHit->SetDE(edep);
  newHit->SetVertexKineticEnergy(theTrack->GetKineticEnergy());

  G4String curVolname = theTrack->GetVolume()->GetName();
  
  newHit->SetVolume(curVolname);

  staHitsCollection->insert( newHit );

  return true;
}

void GenericSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel>0)
  {
    G4int NbHits = staHitsCollection->entries();
    G4cout << "\n-------->Hits Collection: in this event there are " << NbHits
           << " hits at detector station 1: " << G4endl;
    for (G4int i=0;i<NbHits;i++)
      (*staHitsCollection)[i]->Print();
  } 
}
