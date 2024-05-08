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

  OnlMonServer::SetHost("192.168.24.211"); // default = localhost

  OnlMonClientList_t list_omc;
  list_omc.push_back(new OnlMonMainDaq());
  list_omc.push_back(new OnlMonTrigSig());
  list_omc.push_back(new OnlMonTrigNim());
  list_omc.push_back(new OnlMonTrigV1495("rs_FPGA1_NIM_top.txt", "", "rs_FPGA1_NIM_bottom.txt", ""));
  list_omc.push_back(new OnlMonTrigEP   ("rs_FPGA1_NIM_top.txt", "", "rs_FPGA1_NIM_bottom.txt", ""));
  list_omc.push_back(new OnlMonQie());
  list_omc.push_back(new OnlMonV1495(OnlMonV1495::H1X, 1));
  list_omc.push_back(new OnlMonV1495(OnlMonV1495::H2X, 1));
  list_omc.push_back(new OnlMonV1495(OnlMonV1495::H3X, 1));
  list_omc.push_back(new OnlMonV1495(OnlMonV1495::H4X, 1));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H1X ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H2X ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H3X ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4X ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H1Y ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H2Y ));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4Y1));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4Y2));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::DP1T));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::DP1B));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::DP2T));
  list_omc.push_back(new OnlMonHodo (OnlMonHodo ::DP2B));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4T  ));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4B  ));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4Y1L));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4Y1R));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4Y2L));
  list_omc.push_back(new OnlMonH4   (OnlMonH4   ::H4Y2R));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D0 ));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D2 ));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D3p));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D3m));
  list_omc.push_back(new OnlMonProp (OnlMonProp ::P1 ));
  list_omc.push_back(new OnlMonProp (OnlMonProp ::P2 ));

  OnlMonUI* ui = new OnlMonUI(&list_omc);
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
