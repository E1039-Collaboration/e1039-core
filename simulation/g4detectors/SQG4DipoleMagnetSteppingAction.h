#ifndef SQG4DipoleMagnetSteppingAction_h
#define SQG4DipoleMagnetSteppingAction_h

#include <g4main/PHG4SteppingAction.h>

class G4VPhysicalVolume;
class SQG4DipoleMagnetDetector;
class PHG4Hit;
class PHG4HitContainer;
class PHParameters;
class PHG4Shower;

class SQG4DipoleMagnetSteppingAction: public PHG4SteppingAction
{
public:
  //! constructor
  SQG4DipoleMagnetSteppingAction(SQG4DipoleMagnetDetector*, const PHParameters* parameters);

  //! destructor
  virtual ~SQG4DipoleMagnetSteppingAction();

  //! stepping action
  virtual bool UserSteppingAction(const G4Step*, bool);

  //! reimplemented from base class
  virtual void SetInterfacePointers(PHCompositeNode*);

private:
  //! pointer to the detector
  SQG4DipoleMagnetDetector *detector_;
  const PHParameters *params;
  //! pointer to hit container
  PHG4HitContainer *hits_;
  PHG4Hit *hit;
  PHG4Shower *saveshower;
  G4VPhysicalVolume *savevolpre;
  G4VPhysicalVolume *savevolpost;
  int savetrackid;
  int saveprestepstatus;
  int savepoststepstatus;
  int active;
  int IsBlackHole;
  int use_g4_steps;

  //FMag track filter settings
  int enable_track_filter;
  double filter_max_slope;
  double filter_min_energy;
};

#endif
