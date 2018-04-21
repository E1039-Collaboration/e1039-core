#include "PHG4CollimatorDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Utils.h>


#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4SubtractionSolid.hh>
#include <Geant4/G4Colour.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4UserLimits.hh>
#include <Geant4/G4VisAttributes.hh>

#include <sstream>

using namespace std;

//_______________________________________________________________
PHG4CollimatorDetector::PHG4CollimatorDetector( PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr):
  PHG4Detector(Node, dnam),
  params(parameters),
  block_physi(nullptr),
  layer(lyr)
{}

//_______________________________________________________________
bool PHG4CollimatorDetector::IsInBlock(G4VPhysicalVolume * volume) const
{
  if (volume == block_physi)
  {
    return true;
  }
  return false;
}

namespace {
	bool PlaceLayeredBox(
			G4LogicalVolume *mother,
			G4UserLimits *g4userlimits,
			bool overlapcheck,
			const std::string& name,
			G4Material *mat_inner,
			double x_inner,
			double y_inner,
			double z_inner,
			G4ThreeVector d_inner,
			G4Material *mat_outer,
			double x_outer,
			double y_outer,
			double z_outer,
			G4ThreeVector d_outer
			)
	{
		G4VSolid *all_solid = new G4Box((name+"_all").c_str(),
				x_outer/2,
				y_outer/2,
				z_outer/2);

		G4VSolid *inner_solid = new G4Box((name+"_inner").c_str(),
				x_inner/2,
				y_inner/2,
				z_inner/2+0.1*cm);

		G4VSolid * outer_solid = new G4SubtractionSolid((name+"_outer").c_str(),
				all_solid,
				inner_solid,
				0,
				d_inner - d_outer
				);

		G4VisAttributes *vis_outer = new G4VisAttributes();
		PHG4Utils::SetColour(vis_outer, mat_outer->GetName());
		vis_outer->SetVisibility(true);
		vis_outer->SetForceSolid(true);

		G4LogicalVolume *outer_logic = new G4LogicalVolume(outer_solid,
				mat_outer,
				(name+"_outer").c_str(),
				nullptr, nullptr, g4userlimits);

		outer_logic->SetVisAttributes(vis_outer);

		new G4PVPlacement(
				0,
				d_outer,
				outer_logic,
				(name+"_outer").c_str(),
				mother, 0, false, overlapcheck);

		G4VisAttributes *vis_inner = new G4VisAttributes();
		PHG4Utils::SetColour(vis_inner, mat_inner->GetName());
		vis_inner->SetVisibility(false);
		vis_inner->SetForceSolid(false);

		G4LogicalVolume *inner_logic = new G4LogicalVolume(inner_solid,
				mat_inner,
				(name+"_inner").c_str(),
				nullptr, nullptr, g4userlimits);

		inner_logic->SetVisAttributes(vis_inner);

		new G4PVPlacement(
				0,
				d_inner,
				inner_logic,
				(name+"_inner").c_str(),
				mother, 0, false, overlapcheck);

		return true;
	}

	bool PlaceHollowdBox(
			G4LogicalVolume *mother,
			G4UserLimits *g4userlimits,
			bool overlapcheck,
			const std::string& name,
			G4Material *mat_inner,
			double x_inner,
			double y_inner,
			double z_inner,
			G4ThreeVector d_inner,
			G4Material *mat_outer,
			double x_outer,
			double y_outer,
			double z_outer,
			G4ThreeVector d_outer
			)
	{
		G4VSolid *all_solid = new G4Box((name+"_all").c_str(),
				x_outer/2,
				y_outer/2,
				z_outer/2);

		G4VSolid *inner_solid = new G4Box((name+"_inner").c_str(),
				x_inner/2,
				y_inner/2,
				z_inner/2+0.1*cm);

		G4VSolid * outer_solid = new G4SubtractionSolid((name+"_outer").c_str(),
				all_solid,
				inner_solid,
				0,
				d_inner - d_outer
				);

		G4VisAttributes *vis_outer = new G4VisAttributes();
		PHG4Utils::SetColour(vis_outer, mat_outer->GetName());
		vis_outer->SetVisibility(true);
		vis_outer->SetForceSolid(true);

		G4LogicalVolume *outer_logic = new G4LogicalVolume(outer_solid,
				mat_outer,
				(name+"_outer").c_str(),
				nullptr, nullptr, g4userlimits);

		outer_logic->SetVisAttributes(vis_outer);

		new G4PVPlacement(
				0,
				d_outer,
				outer_logic,
				(name+"_outer").c_str(),
				mother, 0, false, overlapcheck);

		return true;
	}
}
//_______________________________________________________________
void PHG4CollimatorDetector::Construct( G4LogicalVolume* logicWorld )
{
	double inch = 2.54*cm;

	double z = params->get_double_param("size_z")*cm;

	double x1 = 3.08*inch;
	double y1 = 1.37*inch;
	double x2 = 5.00*inch;
	double y2 = 3.81*inch;
	double x3 = 18.00*inch;
	double y3 = 16.78*inch;

	double dy3 = (4.28-8.688)/2*inch;
	double dz3 = params->get_double_param("place_z")*cm;


  G4VSolid *block_solid = new G4Box(G4String(GetName().c_str()),
  		x3/2.,
			y3/2.,
			z/2.);

  double steplimits = params->get_double_param("steplimits") * cm;
  G4UserLimits *g4userlimits = nullptr;
  if (isfinite(steplimits))
  {
    g4userlimits = new G4UserLimits(steplimits);
  }

  G4LogicalVolume *block_logic = new G4LogicalVolume(block_solid,
  		 G4Material::GetMaterial("G4_Galactic"),
			 G4String(GetName().c_str()),
			 nullptr,nullptr,g4userlimits);

  G4VisAttributes* matVis = new G4VisAttributes();
	PHG4Utils::SetColour(matVis, "G4_Galactic");
	matVis->SetVisibility(false);
	matVis->SetForceSolid(false);
  block_logic->SetVisAttributes(matVis);

  G4RotationMatrix *rotm  = new G4RotationMatrix();
  rotm->rotateZ(params->get_double_param("rot_z")*deg);
  block_physi = new G4PVPlacement(
  		rotm,
			G4ThreeVector(0.,dy3,dz3),
			block_logic,
			G4String(GetName().c_str()),
			logicWorld, 0, false, overlapcheck);

  PlaceHollowdBox(
  		block_logic,
			g4userlimits,
			overlapcheck,
			"OuterEnv",
			0,
			x2, y2, z, G4ThreeVector(0.,-dy3,0.),
			G4Material::GetMaterial("G4_Fe"),
			x3, y3, z, G4ThreeVector(0.,0,0.)
			);

  PlaceLayeredBox(
  		block_logic,
			g4userlimits,
			overlapcheck,
			"InnerEnv",
			G4Material::GetMaterial("G4_Galactic"),
			x1, y1, z, G4ThreeVector(0.,-dy3,0.),
			G4Material::GetMaterial("G4_Cu"),
			x2, y2, z, G4ThreeVector(0.,-dy3,0.)
			);
}




























