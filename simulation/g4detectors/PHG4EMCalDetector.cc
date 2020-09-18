#include "PHG4EMCalDetector.h"

#include <phparameter/PHParameters.h>

#include <g4gdml/PHG4GDMLConfig.hh>
#include <g4gdml/PHG4GDMLUtility.hh>

#include <g4main/PHG4Detector.h>       // for PHG4Detector
#include <g4main/PHG4Subsystem.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4PhysicalConstants.hh>
#include <Geant4/G4RotationMatrix.hh>  // for G4RotationMatrix
#include <Geant4/G4String.hh>          // for G4String
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4ThreeVector.hh>  // for G4ThreeVector
#include <Geant4/G4Transform3D.hh>  // for G4Transform3D
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4Types.hh>            // for G4double, G4int
#include <Geant4/G4VPhysicalVolume.hh>  // for G4VPhysicalVolume

#include <TSystem.h>

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>  // for pair, make_pair

class G4VSolid;
class PHCompositeNode;

using namespace std;

//_______________________________________________________________________
PHG4EMCalDetector::PHG4EMCalDetector(PHCompositeNode* node, PHParameters* parameters, const std::string& dnam, const int lyr)
  : PHG4Detector(node, dnam)
  , m_Params(parameters)
  , m_scintLogical(nullptr)
  , m_absorberLogical(nullptr)
  , m_tower_size_x(m_Params->get_double_param("tower_x")*cm)
  , m_tower_size_y(m_Params->get_double_param("tower_y")*cm)
  , m_tower_size_z(m_Params->get_double_param("tower_z")*cm)
  , m_ntowers_x(m_Params->get_int_param("n_towers_x"))
  , m_ntowers_y(m_Params->get_int_param("n_towers_y"))
  , m_npbsc_layers(m_Params->get_int_param("n_layers"))
  , m_ActiveFlag(m_Params->get_int_param("active"))
  , m_AbsorberActiveFlag(m_Params->get_int_param("absorberactive"))
  , m_SuperDetector("NONE")
  , m_Layer(lyr)
{}

//_______________________________________________________________________
int PHG4EMCalDetector::IsInForwardEcal(G4VPhysicalVolume* volume) const
{
  G4LogicalVolume* mylogvol = volume->GetLogicalVolume();
  if(m_ActiveFlag > 0)
  {
    if(m_scintLogical == mylogvol) return 1;
  }
  if(m_AbsorberActiveFlag > 0)
  {
    if(m_absorberLogical == mylogvol) return -1;
  }
  return 0;
}

//_______________________________________________________________________
void PHG4EMCalDetector::Construct(G4LogicalVolume* logicWorld)
{
  if(Verbosity() > 0)
  {
    std::cout << "PHG4EMCalDetector: Begin Construction for " << name << std::endl;
  }

  // Overal envelop volume for the entire detector
  double xLength_enve = m_tower_size_x*m_ntowers_x*cm;
  double yLength_enve = m_tower_size_y*m_ntowers_y*cm;
  double zLength_enve = m_tower_size_z*cm;
  G4VSolid* emcal_enve_solid = new G4Box(Form("%s_enve_solid", name.c_str()), xLength_enve/2., yLength_enve/2., zLength_enve/2.);

  G4Material* mat_Air = G4Material::GetMaterial("G4_AIR");
  G4LogicalVolume* emcal_enve_logic = new G4LogicalVolume(emcal_enve_solid, mat_Air, Form("%s_enve_logic", name.c_str()));

  G4RotationMatrix rot_enve;
  rot_enve.rotateX(m_Params->get_double_param("rot_x")*rad);
  rot_enve.rotateY(m_Params->get_double_param("rot_y")*rad);
  rot_enve.rotateZ(m_Params->get_double_param("rot_z")*rad);

  G4ThreeVector loc_enve;
  loc_enve.setX(m_Params->get_double_param("place_x")*cm);
  loc_enve.setY(m_Params->get_double_param("place_y")*cm);
  loc_enve.setZ(m_Params->get_double_param("place_z")*cm);

  new G4PVPlacement(G4Transform3D(rot_enve, loc_enve), emcal_enve_logic, Form("%s_enve_phys", name.c_str()), logicWorld, false, 0, overlapcheck);

  // Construct a single tower
  G4LogicalVolume* singleTower_logic = ConstructSingleTower();

  // Place calorimeter towers within envelope 
  PlaceTower(emcal_enve_logic, singleTower_logic);
}

G4LogicalVolume* PHG4EMCalDetector::ConstructSingleTower()
{
  if(Verbosity() > 0)
  {
    std::cout << "PHG4EMCalDetector: Build logical volume for a single tower ..." << std::endl;
  }

  // create logical volume for single tower
  G4Material* mat_Air = G4Material::GetMaterial("G4_AIR");
  G4VSolid* singleTower_solid = new G4Box(Form("%s_singleTower_solid", name.c_str()), m_tower_size_x/2., m_tower_size_y/2., m_tower_size_z/2.);
  G4LogicalVolume* singleTower_logic = new G4LogicalVolume(singleTower_solid, mat_Air, Form("%s_singleTower_logic", name.c_str()));

  /* create geometry volumes for scintillator and absorber plates to place inside single_tower */
  // PHENIX EMCal JGL 3/27/2016
  double thickness_layer = m_tower_size_z/(float)m_npbsc_layers;
  double thickness_absorber  = 1.5*mm;     
  double thickness_scint     = 4.0*mm;   
  double thickness_airgap    = thickness_layer - thickness_absorber - thickness_scint;

  G4VSolid* absorber_solid = new G4Box(Form("%_absorberplate_solid", name.c_str()), m_tower_size_x/2., m_tower_size_y/2., thickness_absorber/2.);
  G4VSolid* scint_solid    = new G4Box(Form("%_scintplate_solid", name.c_str()), m_tower_size_x/2., m_tower_size_y/2., thickness_scint/2.);

  G4Material* mat_absorber = G4Material::GetMaterial("G4_Pb");
  G4Material* mat_scint    = G4Material::GetMaterial("G4_POLYSTYRENE");
  m_absorberLogical = new G4LogicalVolume(absorber_solid, mat_absorber, Form("%s_absorberplate_logic", name.c_str()));
  m_scintLogical    = new G4LogicalVolume(scint_solid, mat_scint, Form("%s_scintplace_logic", name.c_str()));

  /* place physical volumes for absorber and scintillator plates */
  double zpos_i = -m_tower_size_z/2. + thickness_absorber/2.;
  for(int i = 0; i < m_npbsc_layers; ++i)
  {
    //Absorber plate first
    new G4PVPlacement(0, G4ThreeVector(0., 0., zpos_i), m_absorberLogical, Form("%s_absplate_%d", name.c_str(), i), singleTower_logic, false, i, overlapcheck);
    //std::cout << " -- creating absorber plate at z = " << zpos_i/cm << std::endl;

    //Scintilator plate second
    zpos_i += (thickness_absorber/2. + thickness_airgap/2. + thickness_scint/2.);
    new G4PVPlacement(0, G4ThreeVector(0., 0., zpos_i), m_scintLogical, Form("%s_sciplate_%d", name.c_str(), i), singleTower_logic, false, i, overlapcheck);
    //std::cout << " -- creating scintillator plate at z = " << zpos_i/cm << std::endl;

    //move to the next layer
    zpos_i += (thickness_scint/2. + thickness_airgap/2. + thickness_absorber/2.);
  }

  return singleTower_logic;
}

void PHG4EMCalDetector::PlaceTower(G4LogicalVolume* envelope_Logic, G4LogicalVolume* singleTower_logic)
{
  int copyNo = 0;
  for(int i = 0; i < m_ntowers_x; ++i)
  {
    double x_pos = -0.5*m_ntowers_x*m_tower_size_x + (i + 0.5)*m_tower_size_x;
    for(int j = 0; j < m_ntowers_y; ++j)
    {
      double y_pos = -0.5*m_ntowers_y*m_tower_size_y + (j + 0.5)*m_tower_size_y;
      if(Verbosity() > Fun4AllBase::VERBOSITY_SOME)
      {
        std::cout << "  -- Placing tower (" << i << ", " << j << ") at x = " << x_pos/cm << " cm, y = " << y_pos/cm << " cm" << std::endl;
      }

      new G4PVPlacement(0, G4ThreeVector(x_pos, y_pos, 0.), singleTower_logic, Form("%s_tower_%d_%d", name.c_str(), i, j), envelope_Logic, false, copyNo++, overlapcheck);
    }
  }
}
