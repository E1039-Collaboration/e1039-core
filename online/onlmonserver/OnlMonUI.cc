#include <iostream>
#include <TGFrame.h>
#include <TGTextView.h>
#include <TGButton.h>
#include "OnlMonClient.h"
#include "OnlMonUI.h"

#include "OnlMonMainDaq.h"
#include "OnlMonTrigSig.h"
#include "OnlMonV1495.h"
#include "OnlMonHodo.h"
#include "OnlMonCham.h"
#include "OnlMonProp.h"
using namespace std;

OnlMonUI::OnlMonUI(OnlMonClientList_t* list) :
  m_auto_cycle(false), m_interval(10), m_thread_id(0)
{
  //for (OnlMonClientList_t::iterator it = list->begin(); it != list->end(); it++) {
  //  m_list_omc.push_back((*it)->Clone());
  //}

  m_list_omc.push_back(new OnlMonMainDaq());
  m_list_omc.push_back(new OnlMonTrigSig());
  m_list_omc.push_back(new OnlMonV1495(OnlMonV1495::H1X, 1));
  m_list_omc.push_back(new OnlMonV1495(OnlMonV1495::H2X, 1));
  m_list_omc.push_back(new OnlMonV1495(OnlMonV1495::H3X, 1));
  m_list_omc.push_back(new OnlMonV1495(OnlMonV1495::H4X, 1));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H1X ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H2X ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H3X ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4X ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H1Y ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H2Y ));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4Y1));
  m_list_omc.push_back(new OnlMonHodo (OnlMonHodo ::H4Y2));
  m_list_omc.push_back(new OnlMonCham (OnlMonCham ::D0 ));
  m_list_omc.push_back(new OnlMonCham (OnlMonCham ::D1 ));
  m_list_omc.push_back(new OnlMonCham (OnlMonCham ::D2 ));
  m_list_omc.push_back(new OnlMonCham (OnlMonCham ::D3p));
  m_list_omc.push_back(new OnlMonCham (OnlMonCham ::D3m));
  m_list_omc.push_back(new OnlMonProp (OnlMonProp ::P1 ));
  m_list_omc.push_back(new OnlMonProp (OnlMonProp ::P2 ));

  BuildInterface();
  StartAutoCycle();

}

void OnlMonUI::BuildInterface()
{
  TGMainFrame* frame = new TGMainFrame(gClient->GetRoot(), 200, 800);

  TGTextView* head = new TGTextView(frame, 200, 50, "E1039 OnlMon Selector");
  frame->AddFrame(head); 

  //TGTextButton*  button[99];
  for (unsigned int ii = 0; ii < m_list_omc.size(); ii++) {
    button[ii] = new TGTextButton(frame, m_list_omc[ii]->Title().c_str());
    button[ii]->Connect("Clicked()", "OnlMonClient", m_list_omc[ii], "StartMonitor()");
    frame->AddFrame(button[ii], new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 2,2,5,5)); // (l, r, t, b) 
  }

  TGCheckButton* check = new TGCheckButton(frame, new TGHotString("Auto-close all canvases"), 99);
  check->SetToolTipText("When checked, all existing canvases are closed by clicking any button above.");
  check->SetState(OnlMonClient::GetClearUsFlag() ? kButtonDown : kButtonUp);
  check->Connect("Toggled(Bool_t)", "OnlMonClient", m_list_omc[0], "SetClearUsFlag(Bool_t)");
  frame->AddFrame(check, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));

  TGCheckButton* cycle = new TGCheckButton(frame, new TGHotString("Auto-cycle all subsystems"), 99);
  cycle->SetToolTipText("When checked, all subsystems are automatically drawn.");
  cycle->SetState(GetAutoCycleFlag() ? kButtonDown : kButtonUp);
  cycle->Connect("Toggled(Bool_t)", "OnlMonUI", this, "SetAutoCycleFlag(Bool_t)");
  frame->AddFrame(cycle, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
 
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
}

void OnlMonUI::RunAutoCycle()
{
  int idx = 0;
  while (true) {
    if (m_auto_cycle) {
      button[idx]->Clicked();
      //m_list_omc[idx]->StartMonitor();
      //++idx;
      if (++idx >= m_list_omc.size()) idx = 0;
    }
    sleep(m_interval);
  }
}
