/// Fun4MainDaq.C:  Fun4all macro to decode the MainDAQ data.
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libonlmonserver)
R__LOAD_LIBRARY(libdecoder_maindaq)
#endif

int Fun4MainDaq(const int run=28700, const int nevent=0)
{
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  const bool is_online  = false;
  const bool is_e1039   = true;
  const bool use_onlmon = true;
  const char* dir_in  = is_e1039 ? "/data2/analysis/kenichi/e1039/codadata" : "/data3/data/mainDAQ";
  const char* dir_out = is_e1039 ? "/data2/analysis/kenichi/e1039/codadata" : ".";

  ostringstream oss;
  oss << setfill('0') 
      << dir_in << "/run_" << setw(6) << run;
  if (is_e1039) oss << "_spin";
  oss << ".dat";
  string fn_in = oss.str();
  oss.str("");
  oss << dir_out << "/run_" << setw(6) << run;
  if (is_e1039) oss << "_spin";
  oss << ".root";
  string fn_out = oss.str();

  OnlMonServer* se = OnlMonServer::instance();
  //se->Verbosity(1);

  Fun4AllEVIOInputManager *in = new Fun4AllEVIOInputManager("MainDaq");
  in->Verbosity(2);
  in->SetOnline(is_online);
  if (! is_e1039) in->EventSamplingFactor(100);
  //if (is_online) in->PretendSpillInterval(20);
  //in->DirParam("/seaquest/production/runs");
  in->fileopen(fn_in);
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", fn_out);
  se->registerOutputManager(out);

  se->registerSubsystem(new DbUpRun());
  se->registerSubsystem(new DbUpSpill());
  se->registerSubsystem(new CalibInTime());
  se->registerSubsystem(new CalibXT());

  if (use_onlmon) { // Register the online-monitoring clients
    se->StartServer();
    se->registerSubsystem(new OnlMonMainDaq());
    se->registerSubsystem(new OnlMonTrigSig());
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
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D0));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D1));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D2));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D3p));
    se->registerSubsystem(new OnlMonCham (OnlMonCham::D3m));
    se->registerSubsystem(new OnlMonProp (OnlMonProp::P1));
    se->registerSubsystem(new OnlMonProp (OnlMonProp::P2));
  }

  se->run(nevent);
  se->End();
  
  delete se;
  cout << "Fun4MainDaq Done!" << endl;
  return 0;
}
