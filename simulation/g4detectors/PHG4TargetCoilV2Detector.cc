#include "PHG4TargetCoilV2Detector.h"

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
PHG4TargetCoilV2Detector::PHG4TargetCoilV2Detector(PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr)
  : PHG4Detector(Node, dnam)
  , params(parameters)
  , cylinder_physi(nullptr)
  , layer(lyr)
{
}

//_______________________________________________________________
bool PHG4TargetCoilV2Detector::IsInCylinder(const G4VPhysicalVolume *volume) const
{
  if (volume == cylinder_physi || volume->GetMotherLogical() == cylinder_physi->GetLogicalVolume())
  {
    return true;
  }
  return false;
}

namespace {

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

	bool PlaceSSMainMag(
			G4LogicalVolume *mother,
			G4UserLimits *g4userlimits,
			bool overlapcheck,
			const std::string& name,
			G4Material *mat,
			double z_offset,
			double in  = 6.0*cm,
			double out = 22.68*cm,
			double z1  = 2.15*cm,
			double z2  = 9.73*cm
			) {

		int n_base     = 2;
		double z_base[]  = {z1, z2};
		double ri_base[] = {in, in};
		double ro_base[] = {out, out};

	  G4VSolid *base_solid = new G4Polycone((name+"_base").c_str(),
	  		0,
				twopi,
				n_base,
				z_base,
				ri_base,
				ro_base);

		int n_sub1     = 6;
		double z_sub1[]  = {3.20*cm, 4.30*cm, 4.30*cm, 7.61*cm, 7.61*cm, z2+0.01*cm};
		double ri_sub1[] = {9.70*cm, 9.70*cm, 6.97*cm, 6.97*cm, in,      in};
		double ro_sub1[] = {11.16*cm,11.16*cm,11.16*cm,11.16*cm,11.16*cm,11.16*cm};

	  G4VSolid *sub1 = new G4Polycone((name+"_sub1").c_str(),
	  		0,
				twopi,
				n_sub1,
				z_sub1,
				ri_sub1,
				ro_sub1);

		int n_sub2     = 2;
		double z_sub2[]  = {2.76*cm,  8.57*cm};
		double ri_sub2[] = {12.47*cm, 12.47*cm};
		double ro_sub2[] = {17.88*cm, 17.88*cm};

	  G4VSolid *sub2 = new G4Polycone((name+"_sub2").c_str(),
	  		0,
				twopi,
				n_sub2,
				z_sub2,
				ri_sub2,
				ro_sub2);

	  G4VSolid * base_sub1 = new G4SubtractionSolid((name+"base_sub1").c_str(),
	  		base_solid,
				sub1,
				0,
				G4ThreeVector(0,0,0)
	  		);

	  G4VSolid * ss_solid = new G4SubtractionSolid((name+"base_sub1_sub2").c_str(),
	  		base_sub1,
				sub2,
				0,
				G4ThreeVector(0,0,0)
	  		);

	  G4VisAttributes *ss_vis = new G4VisAttributes();
		PHG4Utils::SetColour(ss_vis, mat->GetName());
		ss_vis->SetVisibility(true);
		ss_vis->SetForceSolid(true);

	  G4LogicalVolume *ss_logic = new G4LogicalVolume(ss_solid,
	  		mat,
				(name+"_logic").c_str(),
				nullptr, nullptr, g4userlimits);

	  ss_logic->SetVisAttributes(ss_vis);

	  new G4PVPlacement(
	  		0,
				G4ThreeVector(0, 0, z_offset),
				ss_logic,
				(name+"_phys").c_str(),
				mother, 0, false, overlapcheck);
	}

}

//_______________________________________________________________
void PHG4TargetCoilV2Detector::Construct(G4LogicalVolume *logicWorld)
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
#ifdef _DEBUG_ON
	std::cout<< "PHG4TargetCoilV2Detector::Construct: " << Coil << std::endl;
#endif
	// 1/(0.6/7.87+0.2/7.18+0.15/8.902+0.05/10.22) = 7.95 g/cm3
	G4Material* SS316L = new G4Material(name = "SS316L", density = 7.95*g/cm3, ncomponents = 4);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Fe"), 60 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Cr"), 20 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Ni"), 15 * perCent);
	SS316L->AddMaterial(G4Material::GetMaterial("G4_Mo"),  5 * perCent);
#ifdef _DEBUG_ON
	std::cout<< "PHG4TargetCoilV2Detector::Construct: " << SS316L << std::endl;
#endif
  G4VisAttributes *cylinder_vis = new G4VisAttributes();
	PHG4Utils::SetColour(cylinder_vis, "G4_He");
	cylinder_vis->SetVisibility(true);
	cylinder_vis->SetForceSolid(true);
  
  double l = 22.7 * cm;    // length of cylinder
  double gap = 4 *cm;      // gap between cylinders
  double ri = 6.0 * cm;    // inner r of cylinder
  double ro = 22.225 * cm; // inner r of cylinder
  double ts = 0.3 * cm;    // shell thickness

  //double cl = 1e-2 * cm;

  G4VSolid *cylinder_solid = new G4Tubs(G4String(GetName().c_str()),
  		ri,
			ro,
			l/2,
			0*deg,
			twopi);

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
#ifdef _DEBUG_ON
  params->Print();
  rotm->print(std::cout);
#endif
  cylinder_physi = new G4PVPlacement(
  		rotm,
  		G4ThreeVector(
  				params->get_double_param("place_x") * cm,
  				params->get_double_param("place_y") * cm,
					params->get_double_param("place_z") * cm),
					cylinder_logic,
					G4String(GetName().c_str()),
					logicWorld, 0, false, overlapcheck);

  PlaceHollowTube(
  		cylinder_logic,
			0,
			g4userlimits,
			overlapcheck,
			"Shell",
			SS316L, //G4Material::GetMaterial("G4_Cu"),//SS316L,
			ri,
			ro,
			l/2,
			ts,
			0,
			twopi
  );

  // SS structure
  PlaceSSMainMag(
  		cylinder_logic,
			0,
			overlapcheck,
			"SSMainMag",
			SS316L, //G4Material::GetMaterial("G4_Si"),//SS316L,
			-(l+gap)/2,
			ri + 1.01*ts,
			ro - 1.01*ts,
			2.15*cm + 1.01*ts,
			9.73*cm - 1.01*ts
  		);

  double c1_l = 4.5 * cm;
  double c1_ri =  12.5 *cm;
  double c1_ro = 17.2 *cm;
  double c1_t = 0.01*cm; //0.5 *cm;

  PlaceLayeredTube(
  		cylinder_logic,
			-(l+gap)/2 + 5.08*cm,//-7.5 *cm,
			g4userlimits,
			overlapcheck,
			"C1",
			Coil,    //G4Material::GetMaterial("G4_Fe"),//Coil,
			SS316L,  //G4Material::GetMaterial("G4_Cu"),//SS316L,
			c1_ri-c1_t,
			c1_ro+c1_t,
			c1_l/2+c1_t,
			c1_t,
			0,
			twopi
  );


  double c2_l = 5.7 * cm;
  double c2_ri =  7.6 *cm;
  double c2_ro = 9.4 *cm;
  double c2_t = 0.5 *cm;

  PlaceLayeredTube(
  		cylinder_logic,
			-2.8 *cm,
			g4userlimits,
			overlapcheck,
			"C2",
			Coil,
			SS316L,
			c2_ri-c2_t,
			c2_ro+c2_t,
			c2_l/2+c2_t,
			c2_t,
			0,
			twopi
  );


  double c3_l = 1 * cm;
  double c3_ri =  12.7 *cm;
  double c3_ro = 13.7 *cm;
  double c3_t = 0.5 *cm;

  PlaceLayeredTube(
  		cylinder_logic,
			0.9 *cm,
			g4userlimits,
			overlapcheck,
			"C3",
			Coil,
			SS316L,
			c3_ri-c3_t,
			c3_ro+c3_t,
			c3_l/2+c3_t,
			c3_t,
			0,
			twopi
  );
}

int PHG4TargetCoilV2Detector::get_elem_id(const std::string& name) const {
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











