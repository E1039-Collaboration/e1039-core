R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libonlmonserver)

int TestOnlMonClient(const int run_id=3620, const int n_evt=0)
{
  gSystem->Umask(0002);
  string fn_in = UtilOnline::GetDstFilePath(run_id);
  cout << "DST = " << fn_in << endl;
  UtilOnline::SetOnlMonDir("/dev/shm/$USER/onlmon/plots");
  OnlMonServer* se = OnlMonServer::instance();
  se->SetOnline(false);

  ///
  /// Enable only what you want to test
  ///
  se->registerSubsystem(new OnlMonMainDaq());
  se->registerSubsystem(new OnlMonTrigSig());
  //se->registerSubsystem(new OnlMonTrigNim());
  //se->registerSubsystem(new OnlMonQie());
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H1X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H2X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H3X, 1));
  //se->registerSubsystem(new OnlMonV1495(OnlMonV1495::H4X, 1));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H3X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4X));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H1Y));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H2Y));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y1));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::H4Y2));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1T));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP1B));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2T));
  //se->registerSubsystem(new OnlMonHodo (OnlMonHodo::DP2B));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4T));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4B));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1L));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y1R));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2L));
  //se->registerSubsystem(new OnlMonH4   (OnlMonH4::H4Y2R));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D0));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D2));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D3p));
  //se->registerSubsystem(new OnlMonCham (OnlMonCham::D3m));
  //se->registerSubsystem(new OnlMonProp (OnlMonProp::P1));
  //se->registerSubsystem(new OnlMonProp (OnlMonProp::P2));


  Fun4AllInputManager* in = new Fun4AllDstInputManager("DSTIN");
  se->registerInputManager(in);
  in->fileopen(fn_in);
  se->run(n_evt);
  se->End();
  delete se;
  cout << "\nAll finished.\n"
       << "OnlMon output dir = " << UtilOnline::GetOnlMonDir() << endl;
  exit(0);
}
