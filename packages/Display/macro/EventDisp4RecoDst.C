#include <top/G4_Target.C>
#include <top/G4_InsensitiveVolumes.C>
#include <top/G4_SensitiveDetectors.C>
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libdecoder_maindaq)
R__LOAD_LIBRARY(libg4testbench)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libevt_filter)
R__LOAD_LIBRARY(libpheve_display)
R__LOAD_LIBRARY(libpheve_modules)
R__LOAD_LIBRARY(libpheve_interface)

void EventDisp4RecoDst(const int run_id=6111, const string fn_dst="/data4/e1039_data/semi_online_reco/run_6111/spill_1936201/DST.root")
{
  EventDispOfflineUI* ui = new EventDispOfflineUI();

  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(0);
  se->setRun(run_id);

  PHG4Reco *g4Reco = new PHG4Reco();
  se->registerSubsystem(g4Reco);
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  g4Reco->SetWorldShape("G4BOX");
  g4Reco->SetWorldMaterial("G4_AIR"); //G4_Galactic, G4_AIR
  g4Reco->SetPhysicsList("FTFP_BERT");

  SetupInsensitiveVolumes(g4Reco);
  SetupTarget(g4Reco);
  SetupSensitiveDetectors(g4Reco);

  se->registerSubsystem(new EvtFilter());

  /// (width, height, use_fieldmap, use_geofile, field-map name, geo-file name)
  PHEventDisplay* disp = new PHEventDisplay(1920, 1080, false, false, "", "geom.root");
  //disp->set_verbosity(3);
  se->registerSubsystem(disp);
  
  g4Reco->InitRun(se->topNode());
  disp  ->InitRun(se->topNode());

  Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(in);

  ui->Run(run_id, fn_dst);
}
