#include <iostream>
#include <sys/stat.h>
#include <TSystem.h>
#include <TGFrame.h>
#include <TGTextView.h>
#include <TGButton.h>
#include "OnlMonClient.h"
#include "OnlMonUI.h"
using namespace std;

OnlMonUI::OnlMonUI(OnlMonClientList_t* list) :
  m_auto_cycle(false), m_interval(10), m_thread_id(0), m_list_omc(list)
{
  ;
}

void OnlMonUI::Run()
{
  BuildInterface();
  //StartAutoCycle();
}

void OnlMonUI::BuildInterface()
{
  TGMainFrame* frame = new TGMainFrame(gClient->GetRoot(), 200, 900);
  frame->SetWMPosition(0, 0); // Often ignored by the window manager

  string fn_img = gSystem->Getenv("E1039_RESOURCE");
  fn_img += "/image/onlmon_banner.png";
  struct stat st;
  if (stat(fn_img.c_str(), &st) == 0) {
    TGPictureButton* head2 = new TGPictureButton(frame, fn_img.c_str());
    head2->SetCommand("cout << \">> E1039 Online Monitor <<\" << endl;");
    frame->AddFrame(head2);
  } else {
    TGTextView* head = new TGTextView(frame, 200, 40, "E1039 Online Monitor");
    frame->AddFrame(head); 
  }

  TGTextButton*  button[99];
  for (unsigned int ii = 0; ii < m_list_omc->size(); ii++) {
    button[ii] = new TGTextButton(frame, m_list_omc->at(ii)->Title().c_str());
    button[ii]->Connect("Clicked()", "OnlMonClient", m_list_omc->at(ii), "StartMonitor()");
    frame->AddFrame(button[ii], new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 2,2,5,5)); // (l, r, t, b) 
  }

  TGCheckButton* check = new TGCheckButton(frame, new TGHotString("Auto-close all canvases"), 99);
  check->SetToolTipText("When checked, all existing canvases are closed by clicking any button above.");
  check->SetState(OnlMonClient::GetClearUsFlag() ? kButtonDown : kButtonUp);
  check->Connect("Toggled(Bool_t)", "OnlMonClient", m_list_omc->at(0), "SetClearUsFlag(Bool_t)");
  frame->AddFrame(check, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));

  //TGCheckButton* cycle = new TGCheckButton(frame, new TGHotString("Auto-cycle all subsystems"), 99);
  //cycle->SetToolTipText("When checked, all subsystems are automatically drawn.");
  //cycle->SetState(GetAutoCycleFlag() ? kButtonDown : kButtonUp);
  //cycle->Connect("Toggled(Bool_t)", "OnlMonUI", this, "SetAutoCycleFlag(Bool_t)");
  //frame->AddFrame(cycle, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
 
  TGTextButton* fExit = new TGTextButton(frame, "Exit","gApplication->Terminate(0)");
  frame->AddFrame(fExit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,2,5,5));

  frame->SetWindowName("E1039 Online Monitor");
  frame->MapSubwindows();
  frame->Resize(frame->GetDefaultSize());
  frame->MapWindow();
}

void OnlMonUI::StartAutoCycle()
{
  pthread_create(&m_thread_id, NULL, FuncAutoCycle, this);
}

void* OnlMonUI::FuncAutoCycle(void* arg)
{
  OnlMonUI* ui = (OnlMonUI*)arg;
  ui->RunAutoCycle();
  return 0;
}

/// This function does NOT work at present.
/**
 * The call of OnlMonClient::StartMonitor() crashes.
 * The ROOT graphic classes seem not thread-safe since they manage global variables (like gPad).
 * It is hard for me to control when global variables are auto-modified by GUI.
 */
void OnlMonUI::RunAutoCycle()
{
  unsigned int idx = 0;
  while (true) {
    if (m_auto_cycle) {
      //button[idx]->Clicked();
      m_list_omc->at(idx)->StartMonitor();
      //++idx;
      if (++idx >= m_list_omc->size()) idx = 0;
    }
    sleep(m_interval);
  }
}
