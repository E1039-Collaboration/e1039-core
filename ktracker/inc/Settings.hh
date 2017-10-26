#ifndef Settings_h
#define Settings_h 1

#include "G4String.hh"
#include "globals.hh"
#include "G4Event.hh"
#include "G4SteppingManager.hh"
#include "G4Track.hh"
#include "G4Trajectory.hh"
#include "G4TrajectoryPoint.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TrackStatus.hh"
#include "G4VPhysicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4ThreeVector.hh"

class Settings
{
public:
  Settings();

  int seed;			// The seed for the random number generator
  double beamMomentum;		// The momentum of the beam
  double beamCurrent;		// protons/sec of the beam
  bool asciiFieldMap;		// True if the magnetic field is loaded from the ascii files, false if from SQL
  G4String generator;		// The type of event generator running, i.e. gun or dimuon
  int target;			// The material the target is made of
  double energyCut;		// How much energy a particle must have to be recorded.  All particles that cause hits are recorded anyway.
  G4String recordMethod;	// Hits if a hit is required to record an event, Energy if not
  G4String eventPos;		// Where the event is generated, target, dump or both.  Doesn't affect gun generator
  G4String dimuonSource;	// Whether the dimuons come from Drell-Yan, J/Psi, or both
  G4String login;		// The login for the SQL server
  G4String outputFileName;	// The database name that output goes to
  G4String password;		// The password for the SQL server
  G4String fMagName;		// Name of the ascii text file that contains the fmag map
  G4String kMagName;		// Name of the ascii text file that contains the kmag map
  G4String sqlServer;		// Address of the SQL Server, shouldn't need to modify
  int sqlPort;                  // Port of SQL server
  int dimuonRepeat;		// Usually one.  Number of times the same dimuon is generated each event
  bool ironOn;			// True if the magnet iron is in the run
  double trackingEnergyCut;	// The energy threshold a particle has to fall below before GMC considers killing the track
  double trackingZCut;		// No particles to the left of this position will be killed
  double fMagMultiplier;	// Multiplies the strength of FMAG's field
  double kMagMultiplier;	// Multiplies the strength of KMAG's field
  G4String geometrySchema;	// The sql schema that GMC pulls the geometry information from
  G4String magnetSchema;	// The sql schema that GMC pulls the magnetic field information from
  G4String customSchema;        // The sql schema that GMC pulls the event generation information for the custom generator from
  bool pythia_shower;		// If set to false, Pythia doesn't simulate QCD showers
  int bucket_size;

};
#endif
