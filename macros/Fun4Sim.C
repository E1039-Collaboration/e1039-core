
#include <iostream>

using namespace std;

int Fun4Sim(const int nEvents = 1)
{
  const int use_g4steps = 1;
  const double target_l = 7.9; //cm
  const double target_z = (7.9-target_l)/2.; //cm

  const bool gen_gun = false;
  const bool gen_pythia8 = true;
  const bool gen_test = false;

  gSystem->Load("libfun4all");
  gSystem->Load("libg4detectors");
  gSystem->Load("libg4testbench");
  gSystem->Load("libg4eval");
  gSystem->Load("libtruth_eval.so");

  ///////////////////////////////////////////
  // Make the Server
  //////////////////////////////////////////
  Fun4AllServer *se = Fun4AllServer::instance();
  se->Verbosity(100);

  // particle gun
  if(gen_gun) {
    PHG4ParticleGun *gun_beam = new PHG4ParticleGun("GUN_beam");
    gun_beam->set_name("proton");//proton
    gun_beam->set_vtx(0, 0, -400); //-363.32 cm
    gun_beam->set_mom(0, 0, 120);
    TF2 *beam_profile = new TF2("beam_profile",
        //"(((x**2+y**2)<=0.81)*exp(-(x**2+y**2)/0.18))+(((x**2+y**2)>0.81&&(x**2+y**2)<=25&&abs(y)<1.)*0.9*exp(-4.5)/(sqrt(x**2+y**2)))",
        "(((x**2+y**2)<=0.81)*exp(-(x**2+y**2)/0.18))+(((x**2+y**2)>0.81&&(x**2+y**2)<=25)*0.9*exp(-4.5)/(sqrt(x**2+y**2)))",
        -5,5,-5,5);
    //gun_beam->set_beam_profile(beam_profile);
    //se->registerSubsystem(gun_beam);
  }

  if(gen_pythia8) {
    gSystem->Load("libPHPythia8.so");

    PHPythia8 *pythia8 = new PHPythia8();
    pythia8->set_config_file("phpythia8.cfg");
    pythia8->set_vertex_distribution_mean(0, 0, -130, 0);
    se->registerSubsystem(pythia8);

    PHPy8ParticleTrigger* trigger = new PHPy8ParticleTrigger();
    trigger->AddParticles("13,-13");
    pythia8->register_trigger(trigger);

    HepMCNodeReader *hr = new HepMCNodeReader();
    se->registerSubsystem(hr);
  }

  if(gen_test) {
    double mass = 7;
    double cot = 7.4;
    PHG4ParticleGun *gun_mup = new PHG4ParticleGun("GUN_mup");
    gun_mup->set_name("mu+");
    gun_mup->set_vtx(0, 0, -130);
    gun_mup->set_mom(mass/2., 0, mass/2.*cot);
    //gun_mup->set_vtx(0, 0, 500);
    //gun_mup->set_mom(0, 0, 2);
    se->registerSubsystem(gun_mup);

    PHG4ParticleGun *gun_mum = new PHG4ParticleGun("GUN_mum");
    gun_mum->set_name("mu-");
    gun_mum->set_vtx(0, 0, -130);
    gun_mum->set_mom(mass/-2., 0, mass/2.*cot);
    se->registerSubsystem(gun_mum);
  }

  // Fun4All G4 module
  PHG4Reco *g4Reco = new PHG4Reco();
  //g4Reco->G4Seed(123);
  //g4Reco->set_field(5.);
  g4Reco->set_field_map(
      "/e906/app/users/liuk/seaquest/seaquest/geometry/magnetic_fields/tab.Fmag "
      "/e906/app/users/liuk/seaquest/seaquest/geometry/magnetic_fields/tab.Kmag",
      4);
  // size of the world - every detector has to fit in here
  g4Reco->SetWorldSizeX(1000);
  g4Reco->SetWorldSizeY(1000);
  g4Reco->SetWorldSizeZ(5000);
  // shape of our world - it is a tube
  g4Reco->SetWorldShape("G4BOX");
  // this is what our world is filled with
  g4Reco->SetWorldMaterial("G4_Galactic");
  // Geant4 Physics list to use
  g4Reco->SetPhysicsList("FTFP_BERT");


  PHG4E1039InsensSubsystem* insens = new PHG4E1039InsensSubsystem("Insens");
  g4Reco->registerSubsystem(insens);

  gROOT->LoadMacro("G4_Target.C");
  SetupTarget(g4Reco, use_g4steps, target_l, target_z);

  gROOT->LoadMacro("G4_DriftChamber.C");
  SetupDriftChamber(g4Reco);

  PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
  g4Reco->registerSubsystem(truth);

  se->registerSubsystem(g4Reco);

  DPDigitizer *digitizer = new DPDigitizer();
  digitizer->Verbosity(0);
  se->registerSubsystem(digitizer);

  gSystem->Load("libmodule_example.so");
  TestSimAnalyzer *analyzer = new TestSimAnalyzer();
  analyzer->Verbosity(0);
  analyzer->set_hit_container_choice("Vector");
  analyzer->set_out_name("test_analyzer.root");
  se->registerSubsystem(analyzer);

  gSystem->Load("libktracker.so");
  KalmanFastTrackingWrapper *ktracker = new KalmanFastTrackingWrapper();
  ktracker->set_geom_file_name("geom.root");
  ktracker->Verbosity(100);
  se->registerSubsystem(ktracker);

  //TruthEval* eval = new TruthEval("TruthEval","eval.root");
  //eval->target_l = target_l;
  //eval->target_z = target_z;
  //se->registerSubsystem(eval);

  ///////////////////////////////////////////
  // Output
  ///////////////////////////////////////////

  // save a comprehensive  evaluation file
  PHG4DSTReader *ana = new PHG4DSTReader(
      string("DSTReader.root"));
  ana->set_save_particle(true);
  ana->set_load_all_particle(false);
  ana->set_load_active_particle(true);
  ana->set_save_vertex(true);
  ana->AddNode("Coil");
  ana->AddNode("Target");
  //ana->AddNode("Collimator");
  ana->AddNode("C1X");
  ana->AddNode("C2X");
  se->registerSubsystem(ana);

  // input - we need a dummy to drive the event loop
  Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
  se->registerOutputManager(out);

  // a quick evaluator to inspect on hit/particle/tower level

  if (nEvents > 0)
  {
    se->run(nEvents);

    PHGeomUtility::ExportGeomtry(se->topNode(),"geom.root");

    // finish job - close and save output files
    se->End();
    std::cout << "All done" << std::endl;

    // cleanup - delete the server and exit
    delete se;
    gSystem->Exit(0);
  }
  return;
}

PHG4ParticleGun *get_gun(const char *name = "PGUN")
{
  Fun4AllServer *se = Fun4AllServer::instance();
  PHG4ParticleGun *pgun = (PHG4ParticleGun *) se->getSubsysReco(name);
  return pgun;
}
