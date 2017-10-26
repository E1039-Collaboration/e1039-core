#ifndef MCHit_h
#define MCHit_h 1

// Based on ParN02 TrackerHit example

#include "G4Circle.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4VVisManager.hh"
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

class MCHit : public G4VHit
{
  public:

    MCHit();
    ~MCHit();

    inline void* operator new(size_t);
    inline void  operator delete(void*);

  private:
    G4int fPid;
    G4String fName;
    G4ThreeVector fPosition;
    G4double fDE;
    G4String fVolume;
    G4ThreeVector fMomentum;
    G4ThreeVector fVertex;
    G4double fTime;
    G4int fTrackID;
    G4int fParentID;
    G4ThreeVector fVertexMomentumDirection;
    G4double fVertexKineticEnergy;

  public:

    inline G4int GetParticleID() const {return fPid;};
    inline void SetParticleID(G4int particle_id) {fPid=particle_id;};

    inline G4String GetParticleName() const {return fName;};
    inline void SetParticleName(G4String pName) {fName=pName;};

    inline G4ThreeVector GetPosition() const {return fPosition;};
    inline void SetPosition(G4ThreeVector pos) {fPosition=pos;};

    inline G4double GetDE() const {return fDE;};
    inline void SetDE(G4double de) {fDE=de;};

    inline G4String GetVolume() const {return fVolume;};
    inline void SetVolume(G4String volume) {fVolume=volume;};

    inline G4ThreeVector GetMomentum() const {return fMomentum;};
    inline void SetMomentum(G4ThreeVector p) {fMomentum=p;};

    inline G4ThreeVector GetVertex() const {return fVertex;};
    inline void SetVertex(G4ThreeVector v0) {fVertex=v0;};

    inline G4double GetTime() const {return fTime;};
    inline void SetTime(G4double time) {fTime=time;};

    inline G4int GetTrackID() const {return fTrackID;};
    inline void SetTrackID(G4int tid) {fTrackID=tid;};

    inline G4int GetParentID() const {return fParentID;};
    inline void SetParentID(G4int parentID) {fParentID=parentID;};

    inline G4ThreeVector GetVertexMomentumDirection() const {return fVertexMomentumDirection;};
    inline void SetVertexMomentumDirection(G4ThreeVector vmd) {fVertexMomentumDirection=vmd;};

    inline G4double GetVertexKineticEnergy() const {return fVertexKineticEnergy;};
    inline void SetVertexKineticEnergy(G4double vke) {fVertexKineticEnergy=vke;};
};

// vector collection of one type of hits
typedef G4THitsCollection<MCHit> MCHitsCollection;

extern G4Allocator<MCHit> MCHitAllocator;

inline void* MCHit::operator new(size_t)
{
  void* aHit;
  aHit = (void*) MCHitAllocator.MallocSingle();
  return aHit;
}

inline void MCHit::operator delete(void* aHit)
{
  MCHitAllocator.FreeSingle((MCHit*) aHit);
}

#endif
