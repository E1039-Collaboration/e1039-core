#include "PHG4CylinderDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Utils.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <Geant4/G4Colour.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4PhysicalConstants.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4UserLimits.hh>
#include <Geant4/G4VisAttributes.hh>

#include <cmath>
#include <sstream>

using namespace std;

//_______________________________________________________________
PHG4CylinderDetector::PHG4CylinderDetector(PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr)
  : PHG4Detector(Node, dnam)
  , params(parameters)
  , cylinder_physi(nullptr)
  , layer(lyr)
{
}

//_______________________________________________________________
bool PHG4CylinderDetector::IsInCylinder(const G4VPhysicalVolume *volume) const
{
  if (volume == cylinder_physi)
  {
    return true;
  }
  return false;
}

//_______________________________________________________________
void PHG4CylinderDetector::Construct(G4LogicalVolume *logicWorld)
{
	G4Material *TrackerMaterial = nullptr;
	if (params->get_string_param("material").find("Target")
			!= std::string::npos) {
		G4double z;
		G4double a;
		G4String symbol;
		G4String name;
		G4double density;
		G4int ncomponents;
		G4int natoms;

		G4Element *elH  = new G4Element(name="Hydrogen", symbol="H" ,  z=1., a = 1.01 *g/mole);
		G4Element *elHe = new G4Element(name="Helium",   symbol="He" , z=2., a = 4.003*g/mole);
		G4Element *elN  = new G4Element(name="Nitrogen", symbol="N" ,  z=7., a = 14.0 *g/mole);

		//G4Material* sN2 = new G4Material(name = "G4_sN2",   density = 0.25  * g/cm3, ncomponents = 1);
		//sN2->AddElement(elN, natoms = 2);

		G4Material* lHe = new G4Material(name = "G4_lHe",   density = 0.145 * g/cm3, ncomponents = 1);
		lHe->AddElement(elHe, natoms = 1);

		G4Material* sNH3 = new G4Material(name = "G4_sNH3", density = 0.867 * g/cm3, ncomponents = 2);
		sNH3->AddElement(elN, natoms = 1);
		sNH3->AddElement(elH, natoms = 3);

		// M_He = 0.4*0.145 = 0.058 g; M_NH3 = 0.6*0.867 = 0.520 g, M/rho = 0.578
		// f_M_He = 10.0%, f_M_NH3 = 90.0%
		G4Material* Target = new G4Material(name = "Target", density = 0.578 * g/cm3, ncomponents = 2);
		Target->AddMaterial(sNH3, 90 * perCent);
		Target->AddMaterial(lHe,  10 * perCent);

		TrackerMaterial = Target;

		std::cout<< "DEBUG: " << TrackerMaterial << std::endl;
	} else if (params->get_string_param("material").find("Coil")
			!= std::string::npos) {
		G4double z;
		G4double a;
		G4String symbol;
		G4String name;
		G4double density;
		G4int ncomponents;
		G4int natoms;

		G4Element *elHe = new G4Element(name="Helium",   symbol="He" , z=2.,  a = 4.003*g/mole);
		G4Element *elFe = new G4Element(name="Iron",     symbol="Fe" , z=26., a = 55.845*g/mole);


		G4Material* lHe = new G4Material(name = "G4_lHe",   density = 0.145 * g/cm3, ncomponents = 1);
		lHe->AddElement(elHe, natoms = 1);

		G4Material* sFe = new G4Material(name = "G4_sFe",   density = 7.87  * g/cm3, ncomponents = 1);
		sFe->AddElement(elFe, natoms = 1);

		// M_He = 0.5*0.145 = 0.0725 g; M_Fe = 0.5*7.87 = 3.935 g, M/rho = 4.0075
		// f_M_He = 2%, f_M_NH3 = 98%
		G4Material* Coil = new G4Material(name = "Coil", density = 4.0075 * g/cm3, ncomponents = 2);
		Coil->AddMaterial(sFe,  98 * perCent);
		Coil->AddMaterial(lHe,  2  * perCent);

		TrackerMaterial = Coil;

		std::cout<< "DEBUG: " << TrackerMaterial << std::endl;
	} else if (params->get_string_param("material").find("KelF")
			!= std::string::npos) {
		G4double z;
		G4double a;
		G4String symbol;
		G4String name;
		G4double density;
		G4int ncomponents;
		G4int natoms;

		G4Element *elC  = new G4Element(name="Carbon", symbol="C" ,  z=6., a = 12.0107 *g/mole);
                G4Element *elF  = new G4Element(name="Fluorine", symbol="F" ,  z=9., a = 18.9984 *g/mole);
                G4Element *elCl  = new G4Element(name="Chlorine", symbol="Cl" ,  z=17., a = 35.453 *g/mole);

		
                G4Material* sC2ClF3 = new G4Material(name = "G4_sC2ClF3", density = 2.12 * g/cm3, ncomponents = 3);
		sC2ClF3->AddElement(elC, natoms = 2);
		sC2ClF3->AddElement(elCl, natoms = 1);
                sC2ClF3->AddElement(elF, natoms = 3);
		G4Material* KelF = new G4Material(name = "KelF", density = 2.05 * g/cm3, ncomponents = 1);
		KelF->AddMaterial(sC2ClF3, 95 * perCent);
		

		TrackerMaterial = KelF;

		std::cout<< "DEBUG: " << TrackerMaterial << std::endl;
         } else if (params->get_string_param("material").find("Target_ladder")
			!= std::string::npos) {
		G4double z;
		G4double a;
		G4String symbol;
		G4String name;
		G4double density;
		G4int ncomponents;
		G4int natoms;

		G4Element *elAl  = new G4Element(name="Aluminium", symbol="Al" ,  z=13., a = 26.98 *g/mole);
		
                G4Material* sAl = new G4Material(name = "G4_sAl", density = 2.7 * g/cm3, ncomponents = 1);
		sAl->AddElement(elAl, natoms = 1);

		G4Material* Target_ladder = new G4Material(name = "Target_ladder", density = 2.7 * g/cm3, ncomponents = 1);
		Target_ladder->AddMaterial(sAl, 100 * perCent);
		

		TrackerMaterial = Target_ladder;

		std::cout<< "DEBUG: " << TrackerMaterial << std::endl;
          } else if (params->get_string_param("material").find("NMR_coil")
			!= std::string::npos) {
		G4double z;
		G4double a;
		G4String symbol;
		G4String name;
		G4double density;
		G4int ncomponents;
		G4int natoms;

		
		G4Element *elCu  = new G4Element(name="Copper", symbol="Cu", z=29., a = 63.546 *g/mole );
		G4Element *elNi = new G4Element(name="Nickel", symbol="Ni", z=28., a = 58.693 *g/mole );
		
		G4Material* sCu = new G4Material(name = "G4_sCu", density = 8.96 * g/cm3, ncomponents = 1);
		sCu->AddElement(elCu, natoms = 1);

		G4Material* sNi = new G4Material(name = "G4_sNi", density = 8.908 * g/cm3, ncomponents = 1);
		sNi->AddElement(elNi, natoms = 1);

		G4Material* NMR_coil = new G4Material(name = "NMR_coil", density = 8.94 * g/cm3, ncomponents = 2);
		NMR_coil->AddMaterial(sCu, 70 * perCent);
		NMR_coil->AddMaterial(sNi, 30 * perCent);
		

		TrackerMaterial = NMR_coil;

		std::cout<< "DEBUG: " << TrackerMaterial << std::endl;
	} else {
		TrackerMaterial = G4Material::GetMaterial(
				params->get_string_param("material"));
	}

  if (!TrackerMaterial)
  {
    std::cout << "Error: Can not set material" << std::endl;
    exit(-1);
  }

  G4VisAttributes *siliconVis = new G4VisAttributes();
  if (params->get_int_param("blackhole"))
  {
    PHG4Utils::SetColour(siliconVis, "BlackHole");
    siliconVis->SetVisibility(false);
    siliconVis->SetForceSolid(false);
  }
  else
  {
    PHG4Utils::SetColour(siliconVis, params->get_string_param("material"));
    siliconVis->SetVisibility(true);
    siliconVis->SetForceSolid(true);
  }

  // determine length of cylinder using PHENIX's rapidity coverage if flag is true
  double radius = params->get_double_param("radius") * cm;
  double thickness = params->get_double_param("thickness") * cm;
  G4VSolid *cylinder_solid = new G4Tubs(G4String(GetName().c_str()),
                                        radius,
                                        radius + thickness,
                                        params->get_double_param("length") * cm / 2., 0, twopi);
  double steplimits = params->get_double_param("steplimits") * cm;
  G4UserLimits *g4userlimits = nullptr;
  if (isfinite(steplimits))
  {
    g4userlimits = new G4UserLimits(steplimits);
  }

  G4LogicalVolume *cylinder_logic = new G4LogicalVolume(cylinder_solid,
                                                        TrackerMaterial,
                                                        G4String(GetName().c_str()),
                                                        nullptr, nullptr, g4userlimits);
  cylinder_logic->SetVisAttributes(siliconVis);

  G4RotationMatrix *rotm  = new G4RotationMatrix();
  rotm->rotateX(params->get_double_param("rot_x")*deg);
  rotm->rotateY(params->get_double_param("rot_y")*deg);
  rotm->rotateZ(params->get_double_param("rot_z")*deg);
  params->Print();
  rotm->print(std::cout);
  cylinder_physi = new G4PVPlacement(rotm, G4ThreeVector(params->get_double_param("place_x") * cm,
                                                      params->get_double_param("place_y") * cm,
                                                      params->get_double_param("place_z") * cm),
                                     cylinder_logic,
                                     G4String(GetName().c_str()),
                                     logicWorld, 0, false, overlapcheck);
}
