/*  This is the class descriptions for the geometry.
    Version .1 9/18/2004
    Larry Isenhower 
*/

#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4EqMagElectricField.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4ThreeVector.hh"
#include "G4UnionSolid.hh"
#include "G4NistManager.hh"
#include "G4Region.hh"
#include "G4ProductionCuts.hh"
#include "G4Polyhedron.hh"
#include "G4PVReplica.hh"
#include "G4UserLimits.hh"
#include "G4FieldManager.hh"
#include "G4ChordFinder.hh"
#include "G4Mag_UsualEqRhs.hh"
#include "G4PropagatorInField.hh"
#include "G4ClassicalRK4.hh"
#include "G4TransportationManager.hh"
#include "G4SDManager.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4PVPlacement.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string>
#include <mysql.h>
#include "GenericSD.hh"
#include "Field.hh"
#include "globals.hh"

class G4Box;
class G4Tubs;
class G4Trd;
class G4SubtractionSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:

    DetectorConstruction(Settings*);
    ~DetectorConstruction();

    G4VPhysicalVolume* Construct();

  public:

    GenericSD* staSD;

    G4Material* targetMat;
    G4Material* defaultMagnetMat;
    G4LogicalVolume* targetVolume;
    G4LogicalVolume* magnetVolume;

    G4double targetLength;
    G4double targetCenter;
    G4double targetRadius;
    G4double fmagCenter;
    G4double fmagLength;
    G4double z1v, z2v, z1h, z2h;

    vector<G4Element*> elementVec;
    vector<G4Material*> materialVec;
    vector<G4VSolid*> solidVec;
    vector<G4LogicalVolume*> logicalVolumeVec;
    vector<G4RotationMatrix*> rotationMatrixVec;

    const G4Material* GetTargetMaterial() {return targetMat;};
    const G4double GetTargetRadius() {return targetRadius;};
    const G4double GetTargetLength() {return targetLength;};
    const G4double GetTargetCenter() {return targetCenter;};
    const G4double GetFmagCenter() {return fmagCenter;};
    const G4double GetFmagLength() {return fmagLength;};

    void SetTargetMaterial(G4String);
    void IronToggle(bool);
    void ReloadMagField();

  private:

    G4VPhysicalVolume* physiWorld;
    int AssignAttributes(G4VPhysicalVolume*);
    MYSQL* con;
    Settings* mySettings;
};

#endif
