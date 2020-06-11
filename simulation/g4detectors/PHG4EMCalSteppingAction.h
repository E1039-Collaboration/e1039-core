#ifndef PHG4EMCalSteppingAction_H
#define PHG4EMCalSteppingAction_H

#include <g4main/PHG4SteppingAction.h>

#include <Geant4/G4TouchableHandle.hh>

class G4Step;
class G4VPhysicalVolume;
class PHCompositeNode;
class PHG4EMCalDetector;
class PHG4Hit;
class PHG4HitContainer;
class PHG4Shower;
class PHParameters;

class PHG4EMCalSteppingAction: public PHG4SteppingAction
{
public:
  //! constructor
  PHG4EMCalSteppingAction(PHG4EMCalDetector*, const PHParameters* parameters);

  //! destroctor
  virtual ~PHG4EMCalSteppingAction();

  //! stepping action
  virtual bool UserSteppingAction(const G4Step*, bool);

  //! reimplemented from base class
  virtual void SetInterfacePointers(PHCompositeNode*);

private:
  //! pointer to the detector
  PHG4EMCalDetector* m_Detector;

  //! pointer to hit container
  PHG4HitContainer* m_SignalHitContainer;
  PHG4HitContainer* m_AbsorberHitContainer;
  PHG4HitContainer* m_CurrentHitContainer;
  PHG4Hit* m_Hit;
  PHG4Shower* m_CurrentShower;

  const PHParameters* m_Params;

  int m_IsActive;
  int m_IsAbsorberActive;
  int m_IsBlackHole;
  int m_AbsorberTruth;
  int m_LightScintModel;

  int m_nTowersX;
  int m_nTowersY;
};

#endif  // PHG4EMCalSteppingAction_H
