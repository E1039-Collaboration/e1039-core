#include "PHG4EMCalSteppingAction.h"
#include "PHG4EMCalDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Hit.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hitv1.h>
#include <g4main/PHG4Shower.h>

#include <g4main/PHG4SteppingAction.h>  // for PHG4SteppingAction
#include <g4main/PHG4TrackUserInfoV1.h>

#include <phool/getClass.h>

#include <Geant4/G4ReferenceCountedHandle.hh>  // for G4ReferenceCountedHandle
#include <Geant4/G4Step.hh>
#include <Geant4/G4StepPoint.hh>   // for G4StepPoint
#include <Geant4/G4StepStatus.hh>  // for fGeomBoundary, fAtRest...
#include <Geant4/G4String.hh>      // for G4String
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4ThreeVector.hh>  // for G4ThreeVector
#include <Geant4/G4TouchableHandle.hh>
#include <Geant4/G4Track.hh>                  // for G4Track
#include <Geant4/G4TrackStatus.hh>            // for fStopAndKill
#include <Geant4/G4Types.hh>                  // for G4double
#include <Geant4/G4VPhysicalVolume.hh>        // for G4VPhysicalVolume
#include <Geant4/G4VTouchable.hh>             // for G4VTouchable
#include <Geant4/G4VUserTrackInformation.hh>  // for G4VUserTrackInformation

#include <iostream>
#include <string>  // for basic_string, operator+

class PHCompositeNode;

using namespace std;

//____________________________________________________________________________..
PHG4EMCalSteppingAction::PHG4EMCalSteppingAction(PHG4EMCalDetector* detector, const PHParameters* parameters)
  : m_Detector(detector)
  , m_SignalHitContainer(nullptr)
  , m_AbsorberHitContainer(nullptr)
  , m_Params(parameters)
  , m_CurrentHitContainer(nullptr)
  , m_Hit(nullptr)
  , m_CurrentShower(nullptr)
  , m_IsActive(m_Params->get_int_param("active"))
  , m_IsAbsorberActive(m_Params->get_int_param("absorberactive"))
  , m_AbsorberTruth(0)
  , m_LightScintModel(1)
  , m_IsBlackHole(m_Params->get_int_param("blackhole"))
  , m_nTowersX(m_Params->get_int_param("n_towers_x"))
  , m_nTowersY(m_Params->get_int_param("n_towers_y"))
{
}

PHG4EMCalSteppingAction::~PHG4EMCalSteppingAction()
{
  // if the last hit was a zero energie deposit hit, it is just reset
  // and the memory is still allocated, so we need to delete it here
  // if the last hit was saved, hit is a nullptr pointer which are
  // legal to delete (it results in a no operation)
  delete m_Hit;
}

//____________________________________________________________________________..
bool PHG4EMCalSteppingAction::UserSteppingAction(const G4Step* aStep, bool)
{
  G4TouchableHandle touch = aStep->GetPreStepPoint()->GetTouchableHandle();
  G4VPhysicalVolume* volume = touch->GetVolume();

  int whichactive = m_Detector->IsInForwardEcal(volume);
  if(whichactive == 0) return false;

  int towerID = -1;
  int plateID = -1;
  int towerIDX = -1;
  int towerIDY = -1;
  int depth = touch->GetHistoryDepth();
  if(depth >= 2)
  {
    towerID = touch->GetCopyNumber(1);
    plateID = touch->GetCopyNumber(0);

    towerIDY = towerID % m_nTowersY;
    towerIDX = (towerID - towerIDY)/m_nTowersY;
  }
  else  //should not happen, just put here for sanity check
  {
    std::cerr << "EMCal volumn history is wrong for " << name << " with history depth = " << depth << std::endl;
  }

  // Get energy deposited by this step 
  double edep = aStep->GetTotalEnergyDeposit()/GeV;
  double eion = (aStep->GetTotalEnergyDeposit() - aStep->GetNonIonizingEnergyDeposit())/GeV;
  double light_yield = 0;

  // Get pointer to associated Geant4 track 
  const G4Track* aTrack = aStep->GetTrack();

  // if this block stops everything, just put all kinetic energy into edep
  if(m_IsBlackHole > 0)
  {
    edep = aTrack->GetKineticEnergy()/GeV;
    G4Track* killtrack = const_cast<G4Track*>(aTrack);
    killtrack->SetTrackStatus(fStopAndKill);
  }

  if(m_IsActive > 0)
  {
    //Check if the particle is geantino
    bool geantino = (aTrack->GetParticleDefinition()->GetPDGEncoding() == 0 && aTrack->GetParticleDefinition()->GetParticleName().find("geantino") != string::npos);

    G4StepPoint* prePoint = aStep->GetPreStepPoint();
    if(prePoint->GetStepStatus() == fGeomBoundary || prePoint->GetStepStatus() == fUndefined)
    {
      if(m_Hit == nullptr)
      {
        m_Hit = new PHG4Hitv1();
      }
  
      m_Hit->set_scint_id(towerID);

      // Set hit location (tower index) 
      m_Hit->set_index_j(towerIDX);
      m_Hit->set_index_k(towerIDY);
      m_Hit->set_index_l(plateID);

      // Set hit location (space point) 
      m_Hit->set_x(0, prePoint->GetPosition().x()/cm);
      m_Hit->set_y(0, prePoint->GetPosition().y()/cm);
      m_Hit->set_z(0, prePoint->GetPosition().z()/cm);

      // Set hit momentum
      m_Hit->set_px(0, prePoint->GetMomentum().x()/GeV);
      m_Hit->set_py(0, prePoint->GetMomentum().y()/GeV);
      m_Hit->set_pz(0, prePoint->GetMomentum().z()/GeV);
      
      // Set hit time 
      m_Hit->set_t(0, prePoint->GetGlobalTime()/nanosecond);

      //set the track ID
      m_Hit->set_trkid(aTrack->GetTrackID());

      // set intial energy deposit 
      m_Hit->set_edep(0.);
      m_Hit->set_eion(0.);

      // Now add the hit to the hit collection 
      // here we do things which are different between scintillator and absorber hits
      if(whichactive > 0)
      {
        m_CurrentHitContainer = m_SignalHitContainer;
        m_Hit->set_light_yield(0.);  // for scintillator only, initialize light yields
      }
      else
      {
        m_CurrentHitContainer = m_AbsorberHitContainer;
      }

      // here we set what is common for scintillator and absorber hits
      if(G4VUserTrackInformation* p = aTrack->GetUserInformation())
      {
        if(PHG4TrackUserInfoV1* pp = dynamic_cast<PHG4TrackUserInfoV1*>(p))
        {
          m_Hit->set_trkid(pp->GetUserTrackId());
          m_Hit->set_shower_id(pp->GetShower()->get_id());
          m_CurrentShower = pp->GetShower();
        }
      }
    }

    if(whichactive > 0)
    {
      light_yield = eion;
      if(m_LightScintModel > 0)
      {
        light_yield = GetVisibleEnergyDeposition(aStep);  //for scintillator only, calculate light yields
      }
    }

    // Update exit values- will be overwritten with every step until
    // we leave the volume or the particle ceases to exist 
    G4StepPoint* postPoint = aStep->GetPostStepPoint();
    m_Hit->set_x(1, postPoint->GetPosition().x()/cm);
    m_Hit->set_y(1, postPoint->GetPosition().y()/cm);
    m_Hit->set_z(1, postPoint->GetPosition().z()/cm);
    m_Hit->set_t(1, postPoint->GetGlobalTime()/nanosecond);

    m_Hit->set_px(1, postPoint->GetMomentum().x()/GeV);
    m_Hit->set_py(1, postPoint->GetMomentum().y()/GeV);
    m_Hit->set_pz(1, postPoint->GetMomentum().z()/GeV);

    // sum up the energy to get total deposited 
    m_Hit->set_edep(m_Hit->get_edep() + edep);
    m_Hit->set_eion(m_Hit->get_eion() + eion);
    if(whichactive > 0)
    {
      m_Hit->set_light_yield(m_Hit->get_light_yield() + light_yield);
    }

    if(geantino)
    {
      m_Hit->set_edep(-1.);  // only energy=0 g4hits get dropped, this way geantinos survive the g4hit compression
      m_Hit->set_eion(-1.);
      if(whichactive > 0)
      {
        m_Hit->set_light_yield(-1.);
      }
    }

    if(edep > 0 && (whichactive > 0 || m_AbsorberTruth > 0))
    {
      if(G4VUserTrackInformation* p = aTrack->GetUserInformation())
      {
        if(PHG4TrackUserInfoV1* pp = dynamic_cast<PHG4TrackUserInfoV1*>(p))
        {
          pp->SetKeep(1);  // we want to keep the track
        }
      }
    }

    if(postPoint->GetStepStatus() == fGeomBoundary || postPoint->GetStepStatus() == fWorldBoundary ||
       postPoint->GetStepStatus() == fAtRestDoItProc || aTrack->GetTrackStatus() == fStopAndKill)
    {
      // save only hits with energy deposit (or -1 for geantino)
      if(m_Hit->get_edep() > 0.)
      {
        m_CurrentHitContainer->AddHit(m_Detector->get_Layer(), m_Hit);
        if(m_CurrentShower != nullptr)
        {
          m_CurrentShower->add_g4hit_id(m_CurrentHitContainer->GetID(), m_Hit->get_hit_id());
        }
        // ownership has been transferred to container, set to null
        // so we will create a new hit for the next track
        m_Hit = nullptr;
      }
      else
      {
        // if this hit has no energy deposit, just reset it for reuse
        // this means we have to delete it in the dtor. If this was
        // the last hit we processed the memory is still allocated
        m_Hit->Reset();
      }
    }

    return true;
  }

  return false;
}

//____________________________________________________________________________..
void PHG4EMCalSteppingAction::SetInterfacePointers(PHCompositeNode* topNode)
{
  std::string hitnodename;
  std::string absorbernodename;

  if(m_Detector->SuperDetector() != "NONE")
  {
    hitnodename = "G4HIT_" + m_Detector->SuperDetector();
    absorbernodename = "G4HIT_ABSORBER_" + m_Detector->SuperDetector();
  }
  else
  {
    hitnodename = "G4HIT_" + m_Detector->GetName();
    absorbernodename = "G4HIT_ABSORBER_" + m_Detector->GetName();
  }

  //now look for the map and grab a pointer to it.
  m_SignalHitContainer   = findNode::getClass<PHG4HitContainer>(topNode, hitnodename);
  m_AbsorberHitContainer = findNode::getClass<PHG4HitContainer>(topNode, absorbernodename);

  // if we do not find the node it's messed up.
  if(m_IsActive > 0 && !m_SignalHitContainer)
  {
    std::cerr << "PHG4EMCalSteppingAction::SetTopNode - unable to find " << hitnodename << std::endl;
  }

  // this is perfectly fine if absorber hits are disabled
  if(m_IsAbsorberActive > 0 && !m_AbsorberHitContainer)
  {
    if(Verbosity() > 0) std::cout << "PHG4EMCalSteppingAction::SetTopNode - unable to find " << absorbernodename << std::endl;
  }
}
