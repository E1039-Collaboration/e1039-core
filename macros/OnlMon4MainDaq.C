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
  gSystem->Load("libdecoder_maindaq.so");
  gSystem->Load("libonlmonserver.so");

  vector<OnlMonClient*> list_omc;
  list_omc.push_back(new OnlMonMainDaq());
  list_omc.push_back(new OnlMonCham(OnlMonCham::D0 ));
  list_omc.push_back(new OnlMonCham(OnlMonCham::D1 ));
  list_omc.push_back(new OnlMonCham(OnlMonCham::D2 ));
  list_omc.push_back(new OnlMonCham(OnlMonCham::D3p));
  list_omc.push_back(new OnlMonCham(OnlMonCham::D3m));
  
  TGMainFrame* frame = new TGMainFrame(gClient->GetRoot(), 400, 800);

  TGTextView* head = new TGTextView(frame, 400, 50, "E1039 OnlMon Selector");
  frame->AddFrame(head); 

  TGTextButton*  button[99];
  for (unsigned int ii = 0; ii < list_omc.size(); ii++) {
    button[ii] = new TGTextButton(frame, list_omc[ii]->Name().c_str());
    button[ii]->Connect("Clicked()", "OnlMonClient", list_omc[ii], "StartMonitor()");
    frame->AddFrame(button[ii], new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 10,10,20,20)); // (l, r, t, b) 
  }
  
  TGTextButton* fExit = new TGTextButton(frame, "Exit","gApplication->Terminate(0)");
  frame->AddFrame(fExit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 10,10,20,20));

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
