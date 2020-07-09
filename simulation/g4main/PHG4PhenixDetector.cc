#include "PHG4PhenixDetector.h"
#include "PHG4Detector.h"
#include "PHG4RegionInformation.h"

#include <phool/recoConsts.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4Element.hh>
#include <Geant4/G4GeometryManager.hh>
#include <Geant4/G4LogicalVolumeStore.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PhysicalVolumeStore.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4Region.hh>
#include <Geant4/G4RegionStore.hh>
#include <Geant4/G4SolidStore.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4VisAttributes.hh>
#include <Geant4/G4FieldManager.hh>

#include <cmath>
#include <iostream>

using namespace std;

//____________________________________________________________________________
PHG4PhenixDetector::PHG4PhenixDetector( void ):
  verbosity(0),
  defaultMaterial(nullptr),
  logicWorld(nullptr),
  physiWorld(nullptr),
  WorldSizeX(1000*cm),
  WorldSizeY(1000*cm),
  WorldSizeZ(1000*cm),
  worldshape("G4BOX"),
  worldmaterial("G4_AIR"),
  ZeroFieldStartZ(99999.*cm),
  lZeroFieldSubWorld(nullptr),
  pZeroFieldSubWorld(nullptr),
  zeroFieldManager(nullptr)
{
}

PHG4PhenixDetector::~PHG4PhenixDetector()
{
  while (detectors_.begin() != detectors_.end())
    {
      delete detectors_.back();
      detectors_.pop_back();
    }
}


//_______________________________________________________________________________________________
G4VPhysicalVolume* PHG4PhenixDetector::Construct()
{
  recoConsts *rc = recoConsts::instance();
  if(verbosity > 0) cout << "PHG4PhenixDetector::Construct." << endl;

  // Clean old geometry, if any
  G4GeometryManager::GetInstance()->OpenGeometry();
  G4PhysicalVolumeStore::GetInstance()->Clean();
  G4LogicalVolumeStore::GetInstance()->Clean();
  G4SolidStore::GetInstance()->Clean();
  if(verbosity > 0) cout << "PHG4PhenixDetector::Construct - cleaning done." << endl;

  //default materials of the World
  //  defaultMaterial  = nist->FindOrBuildMaterial("G4_AIR");

  // World
  G4VSolid* solidWorld = nullptr;
  if(worldshape == "G4BOX")
  {
    solidWorld = new G4Box("World", WorldSizeX/2., WorldSizeY/2., WorldSizeZ/2.);
  }
  else if(worldshape == "G4Tubs")
  {
    solidWorld = new G4Tubs("World", 0., WorldSizeY/2., WorldSizeZ/2., 0., 2.*M_PI);
  }
  else
  {
    cout << "Unknown world shape " << worldshape << endl;
    cout << "implemented are G4BOX, G4Tubs" << endl;
    exit(1);
  }
  rc->set_CharFlag("WorldShape", solidWorld->GetEntityType()); // needed for checks if a particle is inside or outside of our world
  logicWorld = new G4LogicalVolume(solidWorld, G4Material::GetMaterial(worldmaterial), "World");
  logicWorld->SetVisAttributes(G4VisAttributes::Invisible);
  physiWorld = new G4PVPlacement(0, G4ThreeVector(), logicWorld, "World", 0, false, 0);

  G4Region* defaultRegion = (*(G4RegionStore::GetInstance()))[0];
  PHG4RegionInformation* info = new PHG4RegionInformation();
  info->SetWorld();
  defaultRegion->SetUserInformation(info);
  if(verbosity > 0) 
  {
    cout << "PHG4PhenixDetector::Construct " << solidWorld->GetEntityType() << " world "
	       << "material " << logicWorld->GetMaterial()->GetName() << " done." << endl;
  }

  // Zero field zone if activated
  if(worldshape == "G4BOX" && fabs(ZeroFieldStartZ) < 0.5*WorldSizeZ)
  {
    G4double subWorldSizeZ = 0.5*WorldSizeZ - ZeroFieldStartZ;
    G4double subWorldPlaceZ = ZeroFieldStartZ + 0.5*subWorldSizeZ;

    G4VSolid* sZeroFieldSubWorld = new G4Box("ZeroFieldSubWorld", WorldSizeX/2., WorldSizeY/2., subWorldSizeZ/2.);
    lZeroFieldSubWorld = new G4LogicalVolume(sZeroFieldSubWorld, G4Material::GetMaterial(worldmaterial), "ZeroFieldSubWorld");
    pZeroFieldSubWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., subWorldPlaceZ), lZeroFieldSubWorld, "ZeroFieldSubWorld", logicWorld, false, 0, false);
  }
  
  // construct all detectors
  DetectorList::iterator iter = detectors_.begin();
  list<int>::iterator flag = zeroFieldFlags.begin();
  for(; iter != detectors_.end() && flag != zeroFieldFlags.end(); ++iter, ++flag)
  {
    if(*iter)
    {
      (*iter)->Construct((*flag) == 0 ? logicWorld : lZeroFieldSubWorld);
    }
  }

  if(verbosity > 0) cout << "PHG4PhenixDetector::Construct - done." << endl;
  return physiWorld;
}

void PHG4PhenixDetector::ConstructSDandField()
{
  if(lZeroFieldSubWorld != nullptr)
  {
    lZeroFieldSubWorld->SetFieldManager(zeroFieldManager, true);
  }
}

