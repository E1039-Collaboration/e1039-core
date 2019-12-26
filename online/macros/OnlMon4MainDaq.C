/// OnlMon4MainDaq.C:  Macro to launch an online-monitor client for MainDaq.
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libonlmonserver)
#endif

int OnlMon4MainDaq()
{
  if (gROOT->IsBatch()) {
    cout << "ERROR: This macro cannot run in the batch mode (-b).  Abort.\n";
    exit(1);
  }
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  OnlMonServer::SetHost("192.168.24.211"); // default = localhost

  OnlMonUI* ui = new OnlMonUI(0);
  ui->Add(new OnlMonMainDaq());
  ui->Add(new OnlMonTrigSig());
  ui->Add(new OnlMonTrigNim());
  ui->Add(new OnlMonV1495(OnlMonV1495::H1X, 1));
  ui->Add(new OnlMonV1495(OnlMonV1495::H2X, 1));
  ui->Add(new OnlMonV1495(OnlMonV1495::H3X, 1));
  ui->Add(new OnlMonV1495(OnlMonV1495::H4X, 1));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H1X ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H2X ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H3X ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H4X ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H1Y ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H2Y ));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H4Y1));
  ui->Add(new OnlMonHodo (OnlMonHodo ::H4Y2));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4T  ));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4B  ));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4Y1L));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4Y1R));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4Y2L));
  ui->Add(new OnlMonH4   (OnlMonH4   ::H4Y2R));
  ui->Add(new OnlMonCham (OnlMonCham ::D0 ));
  ui->Add(new OnlMonCham (OnlMonCham ::D1 ));
  ui->Add(new OnlMonCham (OnlMonCham ::D2 ));
  ui->Add(new OnlMonCham (OnlMonCham ::D3p));
  ui->Add(new OnlMonCham (OnlMonCham ::D3m));
  ui->Add(new OnlMonProp (OnlMonProp ::P1 ));
  ui->Add(new OnlMonProp (OnlMonProp ::P2 ));

  //ui->SetCycleInterval(5); // default = 10 sec
  //ui->SetAutoCycleFlag(true); // default = false
  ui->Run();
  
  return 0;
}

int OnlMon4MainDaqSingle()
{
  //OnlMonClient* omc = new OnlMonMainDaq();
  OnlMonClient* omc = new OnlMonCham(OnlMonCham::D3p);
  omc->StartMonitor();
  return 0;
}
