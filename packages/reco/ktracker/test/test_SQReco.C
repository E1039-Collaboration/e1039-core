#include <TSystem.h>

R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libfun4all)
R__LOAD_LIBRARY(libg4detectors)
R__LOAD_LIBRARY(libg4eval)
R__LOAD_LIBRARY(libktracker)

/*
This macro takes severl external input files to run:
1. geom.root is the standard geometry dump from running the Fun4Sim macro;
2. e906_run7.opts is provided
3. digit_028692_009.root is E906 run6 data, can be found at /pnfs/e906/production/digit/R009/02/86
*/

int test_SQReco(const int nEvents = 1)
{
  const double FMAGSTR = -1.054;
  const double KMAGSTR = -0.951;

  recoConsts* rc = recoConsts::instance();
  rc->set_DoubleFlag("FMAGSTR", FMAGSTR);
  rc->set_DoubleFlag("KMAGSTR", KMAGSTR);
  rc->Print();

  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(100);

  JobOptsSvc* jobopt_svc = JobOptsSvc::instance();
  jobopt_svc->init("e906_run7.opts");

  GeomSvc::UseDbSvc(false);  //set to true to run E1039 style data
  GeomSvc* geom_svc = GeomSvc::instance();

  SQReco* reco = new SQReco();
  reco->Verbosity(100);
  reco->set_geom_file_name("geom.root");
  reco->set_enable_KF(true); //Kalman filter not needed for the track finding, disabling KF saves a lot of initialization time
  reco->setInputTy(SQReco::E906);    //options are SQReco::E906 and SQReco::E1039
  reco->setFitterTy(SQReco::KFREF);  //not relavant for the track finding
  reco->set_evt_reducer_opt("aoce"); //if not provided, event reducer will be using JobOptsSvc to intialize; to turn off, set it to "none"
  reco->set_enable_eval(true);
  reco->set_eval_file_name("eval.root");
  se->registerSubsystem(reco);

  Fun4AllSRawEventInputManager* in = new Fun4AllSRawEventInputManager("SRawEventIM");
  in->Verbosity(0);
  in->set_tree_name("save");
  in->set_branch_name("rawEvent");
  in->fileopen("digit_028692_009.root");
  se->registerInputManager(in);

  Fun4AllDstOutputManager* out = new Fun4AllDstOutputManager("DSTOUT", "tempout.root");
  se->registerOutputManager(out);
  // out->AddNode("SRawEvent");
  // out->AddNode("SRecEvent");

  se->run(nEvents);
  se->End();

  delete se;
  gSystem->Exit(0);
  return 0;
}