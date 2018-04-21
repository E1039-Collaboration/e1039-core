#include "PHG4PolyconDetector.h"

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
#include <Geant4/G4Polycone.hh>
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4UserLimits.hh>
#include <Geant4/G4VisAttributes.hh>

#include <cmath>
#include <sstream>
#include <algorithm>

using namespace std;

//_______________________________________________________________
PHG4PolyconDetector::PHG4PolyconDetector(PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr)
  : PHG4Detector(Node, dnam)
  , params(parameters)
  , cylinder_physi(nullptr)
  , layer(lyr)
{
}

//_______________________________________________________________
bool PHG4PolyconDetector::IsInCylinder(const G4VPhysicalVolume *volume) const
{
  if (volume == cylinder_physi)
  {
    return true;
  }
  return false;
}

//_______________________________________________________________
void PHG4PolyconDetector::Construct(G4LogicalVolume *logicWorld)
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

  G4int num_z_planes = params->get_int_param("num_z_planes");
  auto z_plane = params->get_vdouble_param("z_plane");
  std::for_each(z_plane.begin(), z_plane.end(), [](double &el){el *= cm;});

  auto r_inner = params->get_vdouble_param("r_inner");
  std::for_each(r_inner.begin(), r_inner.end(), [](double &el){el *= cm;});

  auto r_outer = params->get_vdouble_param("r_outer");
  std::for_each(r_outer.begin(), r_outer.end(), [](double &el){el *= cm;});

  cout << "num_z_planes: " << num_z_planes << endl;
  cout << "z_plane: " << "{ "
  		<< std::for_each(z_plane.cbegin(), z_plane.cend(), [](const double &d){std::cout << d << " ";})
  << "}" << endl;

  G4VSolid *cylinder_solid = new G4Polycone(G4String(GetName().c_str()),
               0*deg,
               twopi,
							 num_z_planes,
							 z_plane.data(),
							 r_inner.data(),
							 r_outer.data());

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
