/// Fun4MainDaq.C:  Fun4all macro to decode the MainDAQ data.
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libdecoder_maindaq)
R__LOAD_LIBRARY(libonlmonserver)
R__LOAD_LIBRARY(libpheve_modules)
R__LOAD_LIBRARY(libktracker)

int Fun4MainDaq(const int run=46, const int nevent=0, const bool is_online=false)
{
  gSystem->Umask(0002);
  const bool output_full_dst = false;
  const bool output_spill_dst = true;
  const bool use_onlmon = true;
  const bool use_evt_disp = true;

  const char* deco_mode = gSystem->Getenv("E1039_DECODER_MODE");
  if (deco_mode && strcmp(deco_mode, "std") == 0) {
    cout << "Output mode = standard." << endl;
  } else {
    cout << "output mode = devel." << endl;
    UtilOnline::UseOutputLocationForDevel();
  }

  DecoStatusDb deco_stat;
  deco_stat.RunStarted(run);

  ostringstream oss;
  oss << UtilOnline::GetCodaFileDir() << "/" << UtilOnline::RunNum2CodaFile(run);
  string fn_in = oss.str();
  oss.str("");
  oss << UtilOnline::GetDstFileDir() << "/" << UtilOnline::RunNum2DstFile(run);
  string fn_out = oss.str();
  gSystem->mkdir(UtilOnline::GetDstFileDir().c_str(), true);

  recoConsts* rc = recoConsts::instance();
  rc->set_IntFlag("RUNNUMBER", run);

  OnlMonServer* se = OnlMonServer::instance();
  //se->Verbosity(1);
  se->SetOnline(is_online);

  Fun4AllEVIOInputManager *in = new Fun4AllEVIOInputManager("MainDaq");
  in->Verbosity(3);
  in->SetOnline(is_online);
  //if (is_online) in->PretendSpillInterval(20);
  in->fileopen(fn_in);
  se->registerInputManager(in);

  se->registerSubsystem(new DbUpRun());
  se->registerSubsystem(new DbUpSpill());
  //se->registerSubsystem(new CalibHodoInTime());
  se->registerSubsystem(new CalibMergeH4());
  //se->registerSubsystem(new CalibDriftDist());

  if (use_onlmon) { // Register the online-monitoring clients
    if (is_online) se->StartServer();
    //OnlMonComm::instance()->SetMaxNumSelSpills(600); // default = 600
    se->registerSubsystem(new OnlMonMainDaq());
    se->registerSubsystem(new OnlMonTrigSig());
    se->registerSubsystem(new OnlMonTrigNim());
    se->registerSubsystem(new OnlMonQie());
    se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H1X, 1));
    se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H2X, 1));
    se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H3X, 1));
    se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H4X, 1));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1X));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2X));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H3X));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4X));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1Y));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2Y));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y1));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y2));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1T));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1B));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2T));
    se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2B));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4T));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4B));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1L));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1R));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2L));
    se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2R));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D0));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D2));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D3p));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D3m));
    se->registerSubsystem(new OnlMonProp (OnlMonProp::P1));
    se->registerSubsystem(new OnlMonProp (OnlMonProp::P2));
  }

  if (output_full_dst) {
    Fun4AllDstOutputManager *om_dst = new Fun4AllDstOutputManager("DSTOUT", fn_out);
    se->registerOutputManager(om_dst);
  }

  if (output_spill_dst) {
    Fun4AllSpillDstOutputManager *om_spdst = new Fun4AllSpillDstOutputManager(UtilOnline::GetDstFileDir(), "SPILLDSTOUT");
    om_spdst->SetSpillStep(100);
    om_spdst->EnableDB();
    se->registerOutputManager(om_spdst);
  }

  if (use_evt_disp) {
    se->registerSubsystem(new EvtDispFilter(1000, 1)); // (step, max per spill)

    oss.str("");
    oss << "/data2/e1039/online/evt_disp";
    gSystem->mkdir(oss.str().c_str(), true);
    oss << "/run_" << setfill('0') << setw(6) << run << "_evt_disp.root";
    Fun4AllDstOutputManager *om_eddst = new Fun4AllDstOutputManager("EDDST", oss.str());
    om_eddst->EnableRealTimeSave();
    om_eddst->AddEventSelector("EvtDispFilter");
    se->registerOutputManager(om_eddst);
  }

  if (is_online) {
    Fun4AllSRawEventOutputManager *om_sraw = new Fun4AllSRawEventOutputManager("/data2/e1039/online");
    om_sraw->Verbosity(10);
    se->registerOutputManager(om_sraw);
  }

  se->run(nevent);
  se->End();
  se->PrintTimer();
  deco_stat.RunFinished(run, 0); // always "result = 0" for now.
  
  delete se;
  cout << "Fun4MainDaq Done!" << endl;
  return 0;
}

void TestOnlMonServer()
{
  gSystem->Load("libonlmonserver.so");
  OnlMonServer* se = OnlMonServer::instance();
  //se->Verbosity(1);
  se->StartServer();
}
