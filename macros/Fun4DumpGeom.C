/** @file
 * @brief Macro to save the geometry info on ROOT file, `geom.root`.
 */
#include <TSystem.h>
#include "G4_Beamline.C"
#include "G4_Target.C"
#include "G4_InsensitiveVolumes.C"
#include "G4_SensitiveDetectors.C"
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libgeom_svc)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4dst)
using namespace std;

/// Macro function to save the geometry info on ROOT file, `geom.root`.
/**
 * t.b.w.
 */
int Fun4DumpGeom(const bool display = true)
{
  // geometry setup
  const bool do_collimator = true;
  const bool do_target     = true;
  const bool do_shielding  = true;
  const bool do_fmag       = true;
  const bool do_kmag       = true;
  const bool do_absorber   = true;
  const bool do_st1DC      = false;
  const bool do_dphodo     = true;

  const double collimator_pos_z = -602.36;
  const double target_coil_pos_z = -300.;
  const double target_l = 7.9; //cm
  const double target_z = (7.9-target_l)/2.; //cm

  const string chamberGas = "SQ_ArCO2";
  const string hodoMat    = "SQ_Scintillator";

  GeomSvc::UseDbSvc(true);
  GeomSvc* geom_svc = GeomSvc::instance();

  //Fun4AllServer - intialize before all other subsystems
  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(99);

  // Fun4All G4 module
  PHG4Reco* g4Reco = new PHG4Reco();
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  // shape of our world - it is a tube
  g4Reco->SetWorldShape("G4BOX");
  // this is what our world is filled with
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR

  // sub-systems
  SetupBeamline(g4Reco, do_collimator, collimator_pos_z);
  SetupTarget(g4Reco, target_coil_pos_z, target_l, target_z, 1, 0);
  SetupInsensitiveVolumes(g4Reco, do_shielding, do_fmag, do_kmag, do_absorber);
  SetupSensitiveDetectors(g4Reco, do_dphodo, do_st1DC, chamberGas, hodoMat);
  se->registerSubsystem(g4Reco);

  // dummy input mananger
  Fun4AllInputManager* in = new Fun4AllDummyInputManager("DUMMY");
  se->registerInputManager(in);

  se->run(1);
  PHGeomUtility::ExportGeomtry(se->topNode(), "geom.root");

  se->End();
  delete se;

  // draw the ROOT geometry
  if(!display) return 0;
  gGeoManager = TGeoManager::Import("geom.root");
  gGeoManager->GetCurrentNode()->GetVolume()->Draw("ogl");

  return 0;
}
