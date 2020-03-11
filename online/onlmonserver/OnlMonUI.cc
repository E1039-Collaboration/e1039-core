#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <TSystem.h>
#include <TGFrame.h>
#include <TGTextView.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <TGDoubleSlider.h>
#include "OnlMonComm.h"
#include "OnlMonClient.h"
#include "OnlMonUI.h"
using namespace std;

OnlMonUI::OnlMonUI(OnlMonClientList_t* list)
  : m_auto_cycle(false)
  , m_interval(10)
  , m_tid1(0)
  , m_tid2(0)
  , m_list_omc(list)
{
  ;
}

void OnlMonUI::Run()
{
  BuildInterface();
  StartBgProc();
}

void OnlMonUI::BuildInterface()
{
  m_fr_main = new TGMainFrame(gClient->GetRoot(), 200, 900);
  m_fr_main->SetWMPosition(0, 0); // Often ignored by the window manager

  string fn_img = gSystem->Getenv("E1039_RESOURCE");
  fn_img += "/image/onlmon_banner.png";
  struct stat st;
  if (stat(fn_img.c_str(), &st) == 0) {
    TGPictureButton* head2 = new TGPictureButton(m_fr_main, fn_img.c_str());
    head2->SetCommand("cout << \">> E1039 Online Monitor <<\" << endl;");
    m_fr_main->AddFrame(head2);
  } else {
    TGTextView* head = new TGTextView(m_fr_main, 200, 40, "E1039 Online Monitor");
    m_fr_main->AddFrame(head); 
  }

  TGTextButton*  button[99];
  for (unsigned int ii = 0; ii < m_list_omc->size(); ii++) {
    button[ii] = new TGTextButton(m_fr_main, m_list_omc->at(ii)->Title().c_str());
    button[ii]->Connect("Clicked()", "OnlMonClient", m_list_omc->at(ii), "StartMonitor()");
    m_fr_main->AddFrame(button[ii], new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 2,2,2,2)); // (l, r, t, b) 
  }

  TGLabel* lbl_sp_sel = new TGLabel(m_fr_main, "- - - Spill Selection - - -");
  m_fr_main->AddFrame(lbl_sp_sel, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,5,2));

  { // Spill selector by none
    TGHorizontalFrame* fr_sp_all = new TGHorizontalFrame(m_fr_main);

    m_rad_sp_all = new TGRadioButton(fr_sp_all, "All ");
    m_rad_sp_all->Connect("Pressed()", "OnlMonUI", this, "HandleSpRadAll()");
    fr_sp_all->AddFrame(m_rad_sp_all, new TGLayoutHints(kLHintsCenterY, 2,2,2,2));

    m_lbl_sp = new TGLabel(fr_sp_all, "? ? ? ? ? ?-? ? ? ? ? ?");
    fr_sp_all->AddFrame(m_lbl_sp, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsRight, 2,2,2,2));

    m_fr_main->AddFrame(fr_sp_all);
  }

  { // Spill selector by number
    TGHorizontalFrame* fr_sp_last = new TGHorizontalFrame(m_fr_main);
    
    m_rad_sp_last = new TGRadioButton(fr_sp_last, "Last   ");
    m_rad_sp_last->Connect("Pressed()", "OnlMonUI", this, "HandleSpRadLast()");
    fr_sp_last->AddFrame(m_rad_sp_last, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

    m_num_sp = new TGNumberEntry(fr_sp_last, OnlMonComm::instance()->GetSpillNum(), 2);
    m_num_sp->Connect("ValueSet(Long_t)", "OnlMonUI", this, "HandleSpLastNum()");
    fr_sp_last->AddFrame(m_num_sp, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2,2,2,2));
 
    TGLabel* lbl_sp_last = new TGLabel(fr_sp_last, "spills");
    fr_sp_last->AddFrame(lbl_sp_last, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2,2,2,2));
    
    m_fr_main->AddFrame(fr_sp_last);
  }

  { // Spill selector by spill range
    m_rad_sp_range = new TGRadioButton(m_fr_main, "Range");
    m_rad_sp_range->Connect("Pressed()", "OnlMonUI", this, "HandleSpRadRange()");
    m_fr_main->AddFrame(m_rad_sp_range, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));  
    
    m_fr_sp_range = new TGVerticalFrame(m_fr_main);
    
    TGHorizontalFrame* fr_sp = new TGHorizontalFrame(m_fr_sp_range);
    
    m_num_sp0 = new TGNumberEntry(fr_sp, 0, 5);
    m_num_sp0->Connect("ValueSet(Long_t)", "OnlMonUI", this, "HandleSpEntLo()");
    fr_sp->AddFrame(m_num_sp0, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX));
    
    TGLabel* lbl_sp2 = new TGLabel(fr_sp, "-");
    fr_sp->AddFrame(lbl_sp2, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX));
    
    m_num_sp1 = new TGNumberEntry(fr_sp, 0, 5);
    m_num_sp1->Connect("ValueSet(Long_t)", "OnlMonUI", this, "HandleSpEntHi()");
    fr_sp->AddFrame(m_num_sp1, new TGLayoutHints(kLHintsCenterY | kLHintsRight));
    
    m_fr_sp_range->AddFrame(fr_sp, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));
    
    m_slider = new TGDoubleHSlider(m_fr_sp_range, 180, 0);
    m_slider->Connect("PositionChanged()", "OnlMonUI", this, "HandleSpSlider()");
    m_slider->SetRange(1, 10);
    m_fr_sp_range->AddFrame(m_slider, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));

    m_fr_main->AddFrame(m_fr_sp_range);
  }


  TGCheckButton* check = new TGCheckButton(m_fr_main, new TGHotString("Auto-close all canvases"), 99);
  check->SetToolTipText("When checked, all existing canvases are closed by clicking any button above.");
  check->SetState(OnlMonClient::GetClearUsFlag() ? kButtonDown : kButtonUp);
  check->Connect("Toggled(Bool_t)", "OnlMonClient", m_list_omc->at(0), "SetClearUsFlag(Bool_t)");
  m_fr_main->AddFrame(check, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,2,2));

  //TGCheckButton* cycle = new TGCheckButton(m_fr_main, new TGHotString("Auto-cycle all subsystems"), 99);
  //cycle->SetToolTipText("When checked, all subsystems are automatically drawn.");
  //cycle->SetState(GetAutoCycleFlag() ? kButtonDown : kButtonUp);
  //cycle->Connect("Toggled(Bool_t)", "OnlMonUI", this, "SetAutoCycleFlag(Bool_t)");
  //m_fr_main->AddFrame(cycle, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 2,2,5,5));
 
  TGTextButton* fExit = new TGTextButton(m_fr_main, "Exit","gApplication->Terminate(0)");
  m_fr_main->AddFrame(fExit, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,2,5,5));

  m_fr_main->SetWindowName("E1039 Online Monitor");
  m_fr_main->MapSubwindows();
  HandleSpRadAll();
  //m_fr_main->Resize(m_fr_main->GetDefaultSize());
  //m_fr_main->MapWindow();
}

void OnlMonUI::StartBgProc()
{
  pthread_create(&m_tid1, NULL, FuncAutoCycle, this);
  pthread_create(&m_tid2, NULL, ExecSpillRangeCheck, this);
}

void* OnlMonUI::FuncAutoCycle(void* arg)
{
  OnlMonUI* ui = (OnlMonUI*)arg;
  ui->ExecAutoCycle();
  return 0;
}

/// This function does NOT work at present.
/**
 * The call of OnlMonClient::StartMonitor() crashes.
 * The ROOT graphic classes seem not thread-safe since they manage global variables (like gPad).
 * It is hard for me to control when global variables are auto-modified by GUI.
 */
void OnlMonUI::ExecAutoCycle()
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

void* OnlMonUI::ExecSpillRangeCheck(void* arg)
{
  OnlMonUI* ui = (OnlMonUI*)arg;
  while (true) {
    ui->UpdateFullSpillRange();
    sleep(6);
  }
  return 0;
}

void OnlMonUI::UpdateFullSpillRange()
{
  OnlMonComm* comm = OnlMonComm::instance();
  comm->ReceiveFullSpillRange();
  if (comm->GetSpillSelectability()) {
    int sp_min, sp_max;
    comm->GetFullSpillRange(sp_min, sp_max);
    
    m_num_sp0->SetLimits(TGNumberFormat::kNELLimitMinMax, sp_min, sp_max);
    m_num_sp1->SetLimits(TGNumberFormat::kNELLimitMinMax, sp_min, sp_max);
    m_slider->SetRange(sp_min, sp_max);
    
    ostringstream oss;
    oss << sp_min << "-" << sp_max;
    m_lbl_sp->SetText(oss.str().c_str());
  } else {
    m_lbl_sp->SetText("(not selectable)");
    m_rad_sp_all  ->SetOn(true );
    m_rad_sp_last ->SetOn(false);
    m_rad_sp_range->SetOn(false);
    m_rad_sp_last ->SetEnabled(false);
    m_rad_sp_range->SetEnabled(false);
  }
}

void OnlMonUI::HandleSpRadAll()
{
  OnlMonComm::instance()->SetSpillMode(OnlMonComm::SP_ALL);
  m_rad_sp_all  ->SetOn(true );
  m_rad_sp_last ->SetOn(false);
  m_rad_sp_range->SetOn(false);
  m_fr_main->HideFrame(m_fr_sp_range);
  m_fr_main->Resize(m_fr_main->GetDefaultSize());
  m_fr_main->MapWindow();
}

void OnlMonUI::HandleSpRadLast()
{
  OnlMonComm::instance()->SetSpillMode(OnlMonComm::SP_LAST);
  m_rad_sp_all  ->SetOn(false);
  m_rad_sp_last ->SetOn(true );
  m_rad_sp_range->SetOn(false);
  m_fr_main->HideFrame(m_fr_sp_range);
  m_fr_main->Resize(m_fr_main->GetDefaultSize());
  m_fr_main->MapWindow();
}

void OnlMonUI::HandleSpRadRange()
{
  UpdateFullSpillRange();

  int sp_min, sp_max;
  OnlMonComm::instance()->GetFullSpillRange(sp_min, sp_max);
  OnlMonComm::instance()->SetSpillRange((sp_min+sp_max)/2, sp_max);
  SyncSpillRange();

  OnlMonComm::instance()->SetSpillMode(OnlMonComm::SP_RANGE);
  m_rad_sp_all  ->SetOn(false);
  m_rad_sp_last ->SetOn(false);
  m_rad_sp_range->SetOn(true );
  m_fr_main->ShowFrame(m_fr_sp_range);
  m_fr_main->Resize(m_fr_main->GetDefaultSize());
  m_fr_main->MapWindow();
}

void OnlMonUI::HandleSpLastNum()
{
  int num = (int)m_num_sp->GetNumber();
  m_num_sp->SetNumber(num); // Make it integer
  OnlMonComm::instance()->SetSpillNum(num);
}

void OnlMonUI::HandleSpEntLo()
{
  OnlMonComm::instance()->SetSpillRangeLow((int)m_num_sp0->GetNumber());
  SyncSpillRange();
}

void OnlMonUI::HandleSpEntHi()
{
  OnlMonComm::instance()->SetSpillRangeHigh((int)m_num_sp1->GetNumber());
  SyncSpillRange();
}

void OnlMonUI::HandleSpSlider()
{
  float sp_lo, sp_hi;
  m_slider->GetPosition(sp_lo, sp_hi);
  OnlMonComm::instance()->SetSpillRange((int)sp_lo, (int)sp_hi);
  SyncSpillRange();
}

void OnlMonUI::SyncSpillRange()
{
  int sp_lo, sp_hi;
  OnlMonComm::instance()->GetSpillRange(sp_lo, sp_hi);
  m_slider->SetPosition(sp_lo, sp_hi);
  m_num_sp0->SetNumber (sp_lo);
  m_num_sp1->SetNumber (sp_hi);
}
