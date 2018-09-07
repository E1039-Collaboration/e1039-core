#include "PHG4SquareTubeDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Utils.h>


#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4Tubs.hh>
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
PHG4SquareTubeDetector::PHG4SquareTubeDetector( PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr):
  PHG4Detector(Node, dnam),
  params(parameters),
  block_physi(nullptr),
  layer(lyr)
{}

//_______________________________________________________________
bool PHG4SquareTubeDetector::IsInBlock(G4VPhysicalVolume * volume) const
{
  if (volume == block_physi)
  {
    return true;
  }
  return false;
}

namespace {
G4VPhysicalVolume* PlaceHollowdBox(
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
		G4VPhysicalVolume* ret = nullptr;

		G4VSolid *all_solid = new G4Box((name+"_all").c_str(),
				x_outer/2,
				y_outer/2,
				z_outer/2);

		G4VSolid *inner_solid = new G4Box((name+"_inner").c_str(),
				x_inner/2,
				y_inner/2,
				z_inner/2+0.0001*cm);

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

		ret = new G4PVPlacement(
				0,
				d_outer,
				outer_logic,
				(name+"_outer").c_str(),
				mother, 0, false, overlapcheck);

		return ret;
	}

G4VPhysicalVolume* PlaceCircleHollowdBox(
			G4LogicalVolume *mother,
			G4UserLimits *g4userlimits,
			bool overlapcheck,
			const std::string& name,
			G4Material *mat_inner,
			double diam_inner,
			double z_inner,
			G4ThreeVector d_inner,
			G4Material *mat_outer,
			double x_outer,
			double y_outer,
			double z_outer,
			G4ThreeVector d_outer
			)
	{
		G4VPhysicalVolume* ret = nullptr;

		G4VSolid *all_solid = new G4Box((name+"_all").c_str(),
				x_outer/2,
				y_outer/2,
				z_outer/2);

		G4VSolid *inner_solid = new G4Tubs((name+"inner").c_str(),
				0, diam_inner/2., z_inner/2.+0.0001*cm,
				-0.0001*CLHEP::twopi, CLHEP::twopi);

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

		ret = new G4PVPlacement(
				0,
				d_outer,
				outer_logic,
				(name+"_outer").c_str(),
				mother, 0, false, overlapcheck);

		return ret;
	}
}
//_______________________________________________________________
void PHG4SquareTubeDetector::Construct( G4LogicalVolume* logicWorld )
{
	double place_x = params->get_double_param("place_x")*cm;
	double place_y = params->get_double_param("place_y")*cm;
	double place_z = params->get_double_param("place_z")*cm;

	double inner_place_x = params->get_double_param("inner_place_x")*cm;
	double inner_place_y = params->get_double_param("inner_place_y")*cm;

	double size_x = params->get_double_param("size_x")*cm;
	double size_y = params->get_double_param("size_y")*cm;
	double size_z = params->get_double_param("size_z")*cm;
	double inner_size_x = params->get_double_param("inner_size_x")*cm;
	double inner_size_y = params->get_double_param("inner_size_y")*cm;

  double steplimits = params->get_double_param("steplimits") * cm;

  G4UserLimits *g4userlimits = nullptr;
  if (isfinite(steplimits))
  {
    g4userlimits = new G4UserLimits(steplimits);
  }

  if(params->get_string_param("hole_type")=="rectangular") {
		block_physi = PlaceHollowdBox(
				logicWorld,
				g4userlimits,
				overlapcheck,
				GetName(),

				0,
				inner_size_x, inner_size_y, size_z,
				G4ThreeVector(place_x+inner_place_x,place_y+inner_place_y,place_z),

				G4Material::GetMaterial(params->get_string_param("material")),
				size_x, size_y, size_z,
				G4ThreeVector(place_x, place_y, place_z)
				);
  }

  if(params->get_string_param("hole_type")=="circle") {
		block_physi = PlaceCircleHollowdBox(
				logicWorld,
				g4userlimits,
				overlapcheck,
				GetName(),

				0,
				params->get_double_param("inner_diameter")*cm, size_z,
				G4ThreeVector(place_x+inner_place_x,place_y+inner_place_y,place_z),

				G4Material::GetMaterial(params->get_string_param("material")),
				size_x, size_y, size_z,
				G4ThreeVector(place_x, place_y, place_z)
				);
  }
}




























