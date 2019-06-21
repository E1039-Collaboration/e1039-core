/// OnlMon4MainDaq.C:  Macro to launch an online-monitor client for MainDaq.
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <TGClient.h>
#include <TGButton.h>
#include <TGFrame.h>
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libonlmonserver)
#endif

#include <vector>

int OnlMon4MainDaq()
{
  if (gROOT->IsBatch()) {
    cout << "ERROR: This macro cannot run in the batch mode (-b).  Abort.\n";
    exit(1);
  }
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  //OnlMonServer::SetHost("seaquestdaq01.fnal.gov"); // default = localhost

  vector<OnlMonClient*> list_omc;
  list_omc.push_back(new OnlMonMainDaq());
  list_omc.push_back(new OnlMonTrigSig());
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
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D0 ));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D1 ));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D2 ));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D3p));
  list_omc.push_back(new OnlMonCham (OnlMonCham ::D3m));
  list_omc.push_back(new OnlMonProp (OnlMonProp ::P1 ));
  list_omc.push_back(new OnlMonProp (OnlMonProp ::P2 ));
  
  TGMainFrame* frame = new TGMainFrame(gClient->GetRoot(), 200, 800);

  TGTextView* head = new TGTextView(frame, 200, 50, "E1039 OnlMon Selector");
  frame->AddFrame(head); 

  TGTextButton*  button[99];
  for (unsigned int ii = 0; ii < list_omc.size(); ii++) {
    button[ii] = new TGTextButton(frame, list_omc[ii]->Title().c_str());
    button[ii]->Connect("Clicked()", "OnlMonClient", list_omc[ii], "StartMonitor()");
    frame->AddFrame(button[ii], new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 2,2,5,5)); // (l, r, t, b) 
  }

  TGCheckButton* check = new TGCheckButton(frame, new TGHotString("Auto-close all canvases"), 99);
  check->SetToolTipText("When checked, all existing canvases are closed by clicking any button above.");
  check->SetState(OnlMonClient::GetClearUsFlag() ? kButtonDown : kButtonUp);
  check->Connect("Toggled(Bool_t)", "OnlMonClient", list_omc[0], "SetClearUsFlag(Bool_t)");
  frame->AddFrame(check, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
 
  TGTextButton* fExit = new TGTextButton(frame, "Exit","gApplication->Terminate(0)");
  frame->AddFrame(fExit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,2,5,5));

  frame->SetWindowName("E1039 Online Monitor");
  frame->MapSubwindows();
  frame->Resize(frame->GetDefaultSize());
  frame->MapWindow();

  return 0;
}

int OnlMon4MainDaqSingle()
{
  //OnlMonClient* omc = new OnlMonMainDaq();
  OnlMonClient* omc = new OnlMonCham(OnlMonCham::D3p);
  omc->StartMonitor();
  return 0;
}
