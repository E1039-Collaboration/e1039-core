#include "PHG4BNLTargetCoilDetector.h"

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
#include <Geant4/G4SubtractionSolid.hh>
#include <Geant4/G4UserLimits.hh>
#include <Geant4/G4VisAttributes.hh>

#include <cmath>
#include <sstream>
#include <algorithm>

using namespace std;

//_______________________________________________________________
PHG4BNLTargetCoilDetector::PHG4BNLTargetCoilDetector(PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr)
  : PHG4Detector(Node, dnam)
  , params(parameters)
  , cylinder_physi(nullptr)
  , layer(lyr)
{
}

//_______________________________________________________________
bool PHG4BNLTargetCoilDetector::IsInCylinder(const G4VPhysicalVolume *volume) const
{
  if (volume == cylinder_physi || volume->GetMotherLogical() == cylinder_physi->GetLogicalVolume())
  {
    return true;
  }
  return false;
}

namespace {
bool PlaceHollowPolycone(
		G4LogicalVolume *mother,
		double place,
		G4UserLimits *g4userlimits,
		bool overlapcheck,
		const std::string& name,
		G4Material *Mouter,
		int o_n_z,
		std::vector<double> o_z_plane,
		std::vector<double> o_r_inner,
		std::vector<double> o_r_outer,
		int i_n_z,
		std::vector<double> i_z_plane,
		std::vector<double> i_r_inner,
		std::vector<double> i_r_outer,
		double phi0 = 0,
		double phi = twopi
		)
{
  G4VSolid *all_solid = new G4Polycone((name+"_all").c_str(),
  		phi0,
			phi,
			o_n_z,
			o_z_plane.data(),
			o_r_inner.data(),
			o_r_outer.data());

  G4VSolid *inner_solid = new G4Polycone((name+"_inner").c_str(),
  		phi0,
			phi,
			i_n_z,
			i_z_plane.data(),
			i_r_inner.data(),
			i_r_outer.data());

  G4VSolid * outer_solid = new G4SubtractionSolid((name+"_outer").c_str(),
  		all_solid,
			inner_solid,
			0,
			G4ThreeVector(0,0,0)
  		);


  G4VisAttributes *vis_outer = new G4VisAttributes();
	PHG4Utils::SetColour(vis_outer, Mouter->GetName());
	vis_outer->SetVisibility(true);
	vis_outer->SetForceSolid(true);

  G4LogicalVolume *outer_logic = new G4LogicalVolume(outer_solid,
  		Mouter,
			(name+"_outer").c_str(),
			nullptr, nullptr, g4userlimits);

  outer_logic->SetVisAttributes(vis_outer);

  new G4PVPlacement(
  		0,
			G4ThreeVector(0, 0, place),
			outer_logic,
			(name+"_outer").c_str(),
			mother, 0, false, overlapcheck);

  return true;
}

bool PlaceHollowTube(
		G4LogicalVolume *mother,
		double place,
		G4UserLimits *g4userlimits,
		bool overlapcheck,
		const std::string& name,
		G4Material *Mouter,
		double in,
		double out,
		double half,
		double t,
		double phi0 = 0,
		double phi = twopi
		)
{
  G4VSolid *all_solid = new G4Tubs((name+"_all").c_str(),
  		in,
			out,
			half,
			phi0,
			phi);

  G4VSolid *inner_solid = new G4Tubs((name+"_inner").c_str(),
  		in+t,
			out-t,
			half-t,
			phi0,
			phi);

  G4VSolid * outer_solid = new G4SubtractionSolid((name+"_outer").c_str(),
  		all_solid,
			inner_solid,
			0,
			G4ThreeVector(0,0,0)
  		);


  G4VisAttributes *vis_outer = new G4VisAttributes();
	PHG4Utils::SetColour(vis_outer, Mouter->GetName());
	vis_outer->SetVisibility(true);
	vis_outer->SetForceSolid(true);

  G4LogicalVolume *outer_logic = new G4LogicalVolume(outer_solid,
  		Mouter,
			(name+"_outer").c_str(),
			nullptr, nullptr, g4userlimits);

  outer_logic->SetVisAttributes(vis_outer);

  new G4PVPlacement(
  		0,
			G4ThreeVector(0, 0, place),
			outer_logic,
			(name+"_outer").c_str(),
			mother, 0, false, overlapcheck);

  return true;
}

bool PlaceLayeredTube(
		G4LogicalVolume *mother,
		double place,
		G4UserLimits *g4userlimits,
		bool overlapcheck,
		const std::string& name,
		G4Material *Minner,
		G4Material *Mouter,
		double in,
		double out,
		double half,
		double t,
		double phi0 = 0,
		double phi = twopi
		)
{
  G4VSolid *all_solid = new G4Tubs((name+"_all").c_str(),
  		in,
			out,
			half,
			phi0,
			phi);

  G4VSolid *inner_solid = new G4Tubs((name+"_inner").c_str(),
  		in+t,
			out-t,
			half-t,
			phi0,
			phi);

  G4VSolid * outer_solid = new G4SubtractionSolid((name+"_outer").c_str(),
  		all_solid,
			inner_solid,
			0,
			G4ThreeVector(0,0,0)
  		);


  G4VisAttributes *vis_inner = new G4VisAttributes();
	PHG4Utils::SetColour(vis_inner, Minner->GetName());
	vis_inner->SetVisibility(true);
	vis_inner->SetForceSolid(true);

  G4LogicalVolume *inner_logic = new G4LogicalVolume(inner_solid,
  		Minner,
			(name+"_inner").c_str(),
			nullptr, nullptr, g4userlimits);

  inner_logic->SetVisAttributes(vis_inner);

  new G4PVPlacement(
  		0,
			G4ThreeVector(0, 0, place),
			inner_logic,
			(name+"_inner").c_str(),
			mother, 0, false, overlapcheck);


  G4VisAttributes *vis_outer = new G4VisAttributes();
	PHG4Utils::SetColour(vis_outer, Mouter->GetName());
	vis_outer->SetVisibility(true);
	vis_outer->SetForceSolid(true);

  G4LogicalVolume *outer_logic = new G4LogicalVolume(outer_solid,
  		Mouter,
			(name+"_outer").c_str(),
			nullptr, nullptr, g4userlimits);

  outer_logic->SetVisAttributes(vis_outer);

  new G4PVPlacement(
  		0,
			G4ThreeVector(0, 0, place),
			outer_logic,
			(name+"_outer").c_str(),
			mother, 0, false, overlapcheck);

  return true;
}
}
//_______________________________________________________________
void PHG4BNLTargetCoilDetector::Construct(G4LogicalVolume *logicWorld)
{
	G4double z;
	G4double a;
	G4String symbol;
	G4String name;
	G4double density;
	G4int ncomponents;
	G4int natoms;

	G4Element *elHe = new G4Element(name="Helium", symbol="He" , z=2.,  a = 4.003*g/mole);
	G4Material* lHe = new G4Material(name = "G4_lHe", density = 0.145 * g/cm3, ncomponents = 1);
	lHe->AddElement(elHe, natoms = 1);

	// 1/(0.45/8.57+0.45/4.506+0.1/8.96) = 6.11 g/cm3
	G4Material* Coil = new G4Material(name = "Coil", density = 6.11*g/cm3, ncomponents = 3);
	Coil->AddMaterial(G4Material::GetMaterial("G4_Nb"), 45 * perCent);
	Coil->AddMaterial(G4Material::GetMaterial("G4_Ti"), 45 * perCent);
	Coil->AddMaterial(G4Material::GetMaterial("G4_Cu"), 10 * perCent);
	std::cout<< "DEBUG: " << Coil << std::endl;

	// 1/(0.6/7.87+0.2/7.18+0.15/8.902+0.05/10.22) = 7.95 g/cm3
	G4Material* SS316L = new G4Material(name = "SS316L", density = 7.95*g/cm3, ncomponents = 4);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Fe"), 60 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Cr"), 20 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Ni"), 15 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Mo"),  5 * perCent);
	std::cout<< "DEBUG: " << SS316L << std::endl;


  G4VisAttributes *cylinder_vis = new G4VisAttributes();
	PHG4Utils::SetColour(cylinder_vis, "G4_He");
	cylinder_vis->SetVisibility(true);
	cylinder_vis->SetForceSolid(true);

  // determine length of cylinder using PHENIX's rapidity coverage if flag is true

	double g1 = 2.3*cm; //gap inner
	double g2 = 3.1*cm; //gap outer
	double zend = 13.8*cm; //z end
	double ri = 4.0*cm;
	double rm = 7.9*cm;
	double ro = 16.5*cm;
  double ts = 0.3 * cm; // shell thickness

  int o_n_z = 3;
  std::vector<double> o_z_plane = {g1, g2, zend};
  std::vector<double> o_r_inner = {ri, ri, ri};
  std::vector<double> o_r_outer = {rm, ro, ro};

  int i_n_z = 3;
  std::vector<double> i_z_plane = {g1+ts, g2, zend-ts};
  std::vector<double> i_r_inner = {ri+ts, ri+ts, ri+ts};
  std::vector<double> i_r_outer = {rm, ro-ts, ro-ts};


  G4VSolid *cylinder_solid = new G4Polycone(G4String(GetName().c_str()),
  		0*degree,
			twopi,
			o_n_z,
			o_z_plane.data(),
			o_r_inner.data(),
			o_r_outer.data());

  double steplimits = params->get_double_param("steplimits") * cm;
  G4UserLimits *g4userlimits = nullptr;
  if (isfinite(steplimits))
  {
    g4userlimits = new G4UserLimits(steplimits);
  }

  G4LogicalVolume *cylinder_logic = new G4LogicalVolume(cylinder_solid,
  		lHe,
  		G4String(GetName().c_str()),
			nullptr, nullptr, g4userlimits);

  cylinder_logic->SetVisAttributes(cylinder_vis);

  G4RotationMatrix *rotm  = new G4RotationMatrix();
  rotm->rotateX(params->get_double_param("rot_x")*deg);
  rotm->rotateY(params->get_double_param("rot_y")*deg);
  rotm->rotateZ(params->get_double_param("rot_z")*deg);
  params->Print();
  rotm->print(std::cout);
  cylinder_physi = new G4PVPlacement(
  		rotm,
  		G4ThreeVector(
  				params->get_double_param("place_x") * cm,
  				params->get_double_param("place_y") * cm,
					params->get_double_param("place_z") * cm),
					cylinder_logic,
					G4String(GetName().c_str()),
					logicWorld, 0, false, overlapcheck);

  PlaceHollowPolycone(
  		cylinder_logic,
			0,
			g4userlimits,
			overlapcheck,
			"Shell",
			SS316L,
			o_n_z,
			o_z_plane,
			o_r_inner,
			o_r_outer,
			i_n_z,
			i_z_plane,
			i_r_inner,
			i_r_outer
  );

  double center = 0*cm;
  double coil_t = 0.5*cm;

  double coil_z1 = 4.0*cm;
  double coil_z2 = 8.3*cm;
  double coil_ri = 11.4*cm;
  double coil_ro = 14.8*cm;

  PlaceLayeredTube(
  		cylinder_logic,
			((coil_z1+coil_z2)/2 - center),
			g4userlimits,
			overlapcheck,
			"C1",
			Coil,
			SS316L,
			coil_ri,
			coil_ro,
			(coil_z2-coil_z1)/2,
			coil_t,
			0,
			twopi
  );

  coil_z1 = 3.3*cm;
  coil_z2 = 8.1*cm;
  coil_ri = 6.9*cm;
  coil_ro = 10.1*cm;

  PlaceLayeredTube(
  		cylinder_logic,
			((coil_z1+coil_z2)/2 - center),
			g4userlimits,
			overlapcheck,
			"C2",
			Coil,
			SS316L,
			coil_ri,
			coil_ro,
			(coil_z2-coil_z1)/2,
			coil_t,
			0,
			twopi
  );

  coil_z1 = 2.9*cm;
  coil_z2 = 4.8*cm;
  coil_ri = 4.5*cm;
  coil_ro = 5.7*cm;

  PlaceLayeredTube(
  		cylinder_logic,
			((coil_z1+coil_z2)/2 - center),
			g4userlimits,
			overlapcheck,
			"C3",
			Coil,
			SS316L,
			coil_ri,
			coil_ro,
			(coil_z2-coil_z1)/2,
			coil_t,
			0,
			twopi
  );
}

int PHG4BNLTargetCoilDetector::get_elem_id(const std::string& name) const {
	int ret = 0;
	if(name.find("C1_inner") != std::string::npos) {
		ret = 10;
	} else if (name.find("C1_outer") != std::string::npos) {
		ret = 11;
	} else if (name.find("C2_inner") != std::string::npos) {
		ret = 20;
	} else if (name.find("C2_outer") != std::string::npos) {
		ret = 21;
	} else if (name.find("C3_inner") != std::string::npos) {
		ret = 30;
	} else if (name.find("C3_outer") != std::string::npos) {
		ret = 31;
	} else if (name.find("Shell") != std::string::npos) {
		ret = 99;
	}
	return ret;
}








