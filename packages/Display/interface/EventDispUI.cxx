#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <TEveManager.h>
#include <TEveBrowser.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <TGLabel.h>
#include <TGNumberEntry.h>
#include <UtilAna/UtilOnline.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <evt_filter/EvtFilter.h>
#include "PHEventDisplay.h"
#include "EventDispUI.h"
using namespace std;

EventDispUI::EventDispUI()
  : m_run(0)
  , m_spill(0)
  , m_n_evt(0)
  , m_i_evt(0)
  , m_fr_main(0)
  , m_fr_menu(0)
  , m_fr_evt_sel(0)
  , m_fr_evt_next(0)
  , m_fr_evt_prev(0)
  , m_lbl_mode(0)
  , m_lbl_run(0)
  , m_lbl_n_evt(0)
  , m_ne_evt_id(0)
  , m_ne_trig(0)
  , m_online_mode(true)
  , m_auto_mode(true)
  , m_tid1(0)
{
  ;
}

std::string EventDispUI::GetDstPath(const int run, const int spill)
{
  if (m_online_mode) {
    return UtilOnline::GetEDDstFilePath(run);
  } else {
    return UtilOnline::GetSpillDstDir(run) + "/" + UtilOnline::GetSpillDstFile(run, spill);
  }
}

bool EventDispUI::FindNewRuns()
{
  int nn = m_list_run.size();
  int run = nn > 0  ? m_list_run[nn-1] : RUN_MIN;
  while (++run < RUN_MAX) {
    string fname = GetDstPath(run, 0);
    if (! gSystem->AccessPathName(fname.c_str())) { // if exists
      m_list_run.push_back(run);
    }
  }
  return m_list_run.size() > nn;
}

bool EventDispUI::FindSpillDSTs()
{
  m_list_spill.clear();
  string dir_dst = UtilOnline::GetSpillDstDir(m_run);
  cout << "\nSpill-level DST directory: " << dir_dst << endl;
  void *dirp = gSystem->OpenDirectory(dir_dst.c_str());
  if (dirp == 0) return false; // The directory does not exist.
  const char* name_char;
  while ((name_char = gSystem->GetDirEntry(dirp))) {
    string name = name_char;
    if (name.length() != 36) continue; // run_005638_spill_001907985_spin.root
    if (name.substr(0, 4) != "run_") continue; // easy check
    int spill = atoi(name.substr(17, 9).c_str());
    m_list_spill.push_back(spill);
  }
  gSystem->FreeDirectory(dirp);
  sort(m_list_spill.begin(), m_list_spill.end());
  return m_list_spill.size() > 0;
}

int EventDispUI::FetchNumEvents(const int run, const int spill)
{
  int ret = -1;
  TFile* file = new TFile(GetDstPath(run, spill).c_str());
  if (file->IsOpen()) {
    TTree* tree = (TTree*)file->Get("T");
    if (tree) ret = tree->GetEntries();
  }
  delete file;
  return ret;
}

int EventDispUI::OpenRunFile(const int run, const int spill)
{
  string fname = GetDstPath(run, spill);
  cout << "EventDispUI::OpenRunFile(): run = " << run << "\n"
       << "  " << fname << endl;
  m_run   = run;
  m_spill = spill;
  m_n_evt = FetchNumEvents(run, spill);
  m_i_evt = 0;
  UpdateLabels();
  Fun4AllInputManager *in = Fun4AllServer::instance()->getInputManager("DSTIN");
  return in->fileopen(fname);
}

void EventDispUI::NextEvent()
{
  if (m_online_mode && m_auto_mode) {
    cout << "NextEvent(): Ignored in auto mode." << endl;
    return;
  }
  if (m_i_evt >= m_n_evt) {
    cout << "NextEvent(): No next event." << endl;
    return;
  }
  Fun4AllServer* se = Fun4AllServer::instance();
  m_i_evt++;
  cout << "Next: " << m_i_evt << " / " << m_n_evt << endl;
  UpdateLabels();
  if (se->run(1, true) != 0) {
    cout << "NextEvent() failed.  No event remains?" << endl;
  }
}

void EventDispUI::PrevEvent()
{
  if (m_online_mode && m_auto_mode) {
    cout << "PrevEvent(): Ignored in auto mode." << endl;
    return;
  }
  if (m_i_evt < 2) {
    cout << "PrevEvent(): No previous events." << endl;
    return;
  }
  int i_new = m_i_evt - 1;
  cout << "Prev: " << i_new << " / " << m_n_evt << endl;
  if (OpenRunFile(m_run, m_spill) != 0) {
    cout << "PrevEvent(): Cannot reopen DST." << endl;
    return;
  }
  m_i_evt = i_new;
  UpdateLabels();
  Fun4AllServer* se = Fun4AllServer::instance();
  if (m_i_evt > 1) se->skip(m_i_evt - 1); // First move to the previous-to-previous event.
  if (se->run(1, true) != 0) { // Then read the previous event.
    cout << "PrevEvent() failed." << endl;
  }
}

void EventDispUI::MoveEvent(const int i_evt)
{
  if (i_evt > m_n_evt) {
    OpenRunFile(m_run, m_spill);
    if (i_evt > m_n_evt) {
      cout << "Unexpected!!" << endl;
      return;
    }
  }
  int i_move = i_evt - m_i_evt;
  m_i_evt = i_evt;
  UpdateLabels();
  Fun4AllServer* se = Fun4AllServer::instance();
  if (i_move > 1) se->skip(i_move - 1); // First move to the previous event.
  if (se->run(1, true) != 0) { // Then read the previous event.
    cout << "MoveEvent() failed." << endl;
  }
}

void EventDispUI::ReqEvtID()
{
  int num = (int)m_ne_evt_id->GetNumberEntry()->GetIntNumber();
  cout << "ReqEvtID: " << num << endl;
  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
  filter->set_event_id_req(num);
}

void EventDispUI::ReqTrig()
{
  int num = (int)m_ne_trig->GetNumberEntry()->GetIntNumber();
  cout << "ReqTrig: " << num << endl;
  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
  filter->set_trigger_req(num);
}

void EventDispUI::ViewTop()
{
  cout << "Top View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_top();
}

void EventDispUI::ViewSide()
{
  cout << "Side View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_side();
}

void EventDispUI::View3D()
{
  cout << "3D View" << endl;
  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
  disp->set_view_3d();
}

void EventDispUI::UpdateLabels()
{
  ostringstream oss;
  oss << (m_online_mode ? "Online" : "Offline") << " Mode";
  m_lbl_mode->SetText(oss.str().c_str());

  oss.str("");
  oss << "Run " << m_run;
  m_lbl_run->SetText(oss.str().c_str());

  oss.str("");
  oss << "Event " << m_i_evt << " / " << m_n_evt;
  m_lbl_n_evt->SetText(oss.str().c_str());
}

void EventDispUI::SetAutoMode(bool value)
{
  m_auto_mode = value;
  UpdateInterface();
}

/**
 * Do all needed before showing GUI.
 */
void EventDispUI::Init(const bool online_mode)
{
  m_online_mode = online_mode;
  if (! FindNewRuns()) {
    cout << "EventDispUI::Init(): Found no run.  Abort." << endl;
    exit(1);
  }
  m_run = m_list_run.back();
  if (! online_mode) {
    cout << "\nHit only 'Enter' to select the last run, " << m_list_run.back() << ".\n"
         << "Or input a run number to select.\n";
    m_run = ReadNumber("Run?");
    cout << "Run = " << m_run << endl;
    if (m_run < 0) m_run = m_list_run.back();
    cout << "Run = " << m_run << endl;

    if (! FindSpillDSTs()) {
      cout << "Found no spill-level DST file.  Abort." << endl;
      exit(2);
    }
    cout << "Available spill-level DST files:\n";
    for (unsigned int ii = 0; ii < m_list_spill.size(); ii++) {
      cout << "  " << setw(9) << m_list_spill[ii];
      if (ii % 5 == 4) cout << "\n";
    }

    cout << "\nHit only 'Enter' to select the last spill, " << m_list_spill.back() << ".\n"
         << "Or input a spill number to select.\n";
    m_spill = ReadNumber("Spill?");
    if (m_spill < 0) m_spill = m_list_spill.back();
  }
}

int EventDispUI::ReadNumber(const std::string msg)
{
  int number;
  while (true) {
    cout << msg << " " << flush;
    char line[64];
    cin.getline(line, 64);
    if (line[0] == '\0') {
      return -1;
    } else if (line[0] == '0') {
      return 0;
    } else if ((number = atoi(line)) > 0) {
      return number;
    } else {
      cout << "  Invalid input." << endl;
    }
  }
}

void EventDispUI::Run()
{
  BuildInterface();
  OpenRunFile(m_run, m_spill);
  MoveEvent(1); // Need process one event here, before calling "FuncNewEventCheck"
  if (m_online_mode) {
    pthread_create(&m_tid1, NULL, FuncNewEventCheck, this);
  }
}

void EventDispUI::BuildInterface()
{
  TEveBrowser* browser = gEve->GetBrowser();
  browser->StartEmbedding(TRootBrowser::kLeft);

  m_fr_main = new TGMainFrame(gClient->GetRoot(), 1000, 600);
  m_fr_main->SetWMPosition(0, 0); // Often ignored by the window manager
  m_fr_main->SetWindowName("Event Display");
  m_fr_main->SetCleanup(kDeepCleanup);

  m_fr_menu = new TGVerticalFrame(m_fr_main);

  TGLabel* lab = 0;
  lab = new TGLabel(m_fr_menu, "- - - Event Info - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

  m_lbl_mode = new TGLabel(m_fr_menu, "Online/Offline Mode");
  m_fr_menu->AddFrame(m_lbl_mode, new TGLayoutHints(kLHintsExpandX, 2,2,2,2));

  m_lbl_run = new TGLabel(m_fr_menu, "Run ??????");
  m_fr_menu->AddFrame(m_lbl_run, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

  m_lbl_n_evt = new TGLabel(m_fr_menu, "Event ?? / ??");
  m_fr_menu->AddFrame(m_lbl_n_evt, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX | kLHintsLeft, 2,2,2,2));

  lab = new TGLabel(m_fr_menu, "- - - Event Navigation - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,10,2));

  { // m_fr_evt_mode
    m_fr_evt_mode = new TGCompositeFrame(m_fr_menu);
    TGCheckButton* check = new TGCheckButton(m_fr_evt_mode, new TGHotString("Auto mode"), 99);
    check->SetToolTipText("When checked, the last sampled event is automatically shown.");
    check->SetState(m_auto_mode ? kButtonDown : kButtonUp);
    check->Connect("Toggled(Bool_t)", "EventDispUI", this, "SetAutoMode(Bool_t)");
    m_fr_evt_mode->AddFrame(check);
    m_fr_menu->AddFrame(m_fr_evt_mode, new TGLayoutHints(kLHintsCenterX, 5, 5, 2, 2));
  }  

  TGTextButton* butt;
  { // m_fr_evt_sel
    m_fr_evt_sel = new TGCompositeFrame(m_fr_menu);

    TGHorizontalFrame* frm1 = 0;
    frm1 = new TGHorizontalFrame(m_fr_evt_sel);
    lab = new TGLabel(frm1, "Event ID");
    frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
    m_ne_evt_id = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
                                    TGNumberFormat::kNEAAnyNumber,
                                    TGNumberFormat::kNELLimitMinMax,
                                    -999999, 999999);
    m_ne_evt_id->Connect("ValueSet(Long_t)", "EventDispUI", this, "ReqEvtID()");
    frm1->AddFrame(m_ne_evt_id, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
    m_fr_evt_sel->AddFrame(frm1);
    
    frm1 = new TGHorizontalFrame(m_fr_evt_sel);
    lab = new TGLabel(frm1, "Trigger");
    frm1->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 3, 4));
    m_ne_trig = new TGNumberEntry(frm1, -1, 9, 999, TGNumberFormat::kNESInteger,
                                  TGNumberFormat::kNEAAnyNumber,
                                  TGNumberFormat::kNELLimitMinMax,
                                  -999, 999);
    m_ne_trig->Connect("ValueSet(Long_t)", "EventDispUI", this, "ReqTrig()");
    frm1->AddFrame(m_ne_trig, new TGLayoutHints(kLHintsCenterY | kLHintsRight, 5, 5, 5, 5));
    m_fr_evt_sel->AddFrame(frm1);

    m_fr_menu->AddFrame(m_fr_evt_sel, new TGLayoutHints(kLHintsCenterX, 5, 5, 2, 2));
  }

  { // m_fr_evt_next
    m_fr_evt_next = new TGCompositeFrame(m_fr_menu);
    
    butt = new TGTextButton(m_fr_evt_next, "Next Event");
    m_fr_evt_next->AddFrame(butt);
    butt->Connect("Clicked()", "EventDispUI", this, "NextEvent()");
    
    m_fr_menu->AddFrame(m_fr_evt_next, new TGLayoutHints(kLHintsCenterX, 5, 5, 2, 2));
  }

  { // m_fr_evt_prev
    m_fr_evt_prev = new TGCompositeFrame(m_fr_menu);
    
    butt = new TGTextButton(m_fr_evt_prev, "Previous Event");
    m_fr_evt_prev->AddFrame(butt);
    butt->Connect("Clicked()", "EventDispUI", this, "PrevEvent()");

    m_fr_menu->AddFrame(m_fr_evt_prev, new TGLayoutHints(kLHintsCenterX, 5, 5, 2, 2));
  }

  lab = new TGLabel(m_fr_menu, "- - - View Navigation - - -");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,10,2));
  
  butt = new TGTextButton(m_fr_menu, "  Top View  ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "ViewTop()");
  
  butt = new TGTextButton(m_fr_menu, " Side View ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "ViewSide()");

  butt = new TGTextButton(m_fr_menu, "   3D View   ");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));
  butt->Connect("Clicked()", "EventDispUI", this, "View3D()");

  lab = new TGLabel(m_fr_menu, "Left-button drag = Rotate");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

  lab = new TGLabel(m_fr_menu, "Middle-button drag = Pan");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

  lab = new TGLabel(m_fr_menu, "Middle-button scroll = Zoom");
  m_fr_menu->AddFrame(lab, new TGLayoutHints(kLHintsCenterY | kLHintsExpandX, 2,2,2,2));

  butt = new TGTextButton(m_fr_menu, "Exit","gApplication->Terminate(0)");
  m_fr_menu->AddFrame(butt, new TGLayoutHints(kLHintsTop | kLHintsExpandX, 2,2,10,2));

  m_fr_main->AddFrame(m_fr_menu);

  m_fr_main->MapSubwindows();
  m_fr_main->Resize();
  m_fr_main->MapWindow();

  browser->StopEmbedding();
  browser->SetTabTitle("Event Control", 0);

  UpdateInterface();
}

void EventDispUI::UpdateInterface()
{
  if (m_online_mode) {
    m_fr_menu->ShowFrame(m_fr_evt_mode);
    m_fr_menu->HideFrame(m_fr_evt_sel);
    if (m_auto_mode) {
      m_fr_menu->HideFrame(m_fr_evt_next);
      m_fr_menu->HideFrame(m_fr_evt_prev);
    } else {
      m_fr_menu->ShowFrame(m_fr_evt_next);
      m_fr_menu->ShowFrame(m_fr_evt_prev);
    }
  } else {
    m_fr_menu->HideFrame(m_fr_evt_mode);
    m_fr_menu->ShowFrame(m_fr_evt_sel);
    m_fr_menu->ShowFrame(m_fr_evt_next);
    m_fr_menu->ShowFrame(m_fr_evt_prev); // HideFrame(m_fr_evt_prev);
  }
  m_fr_menu->Resize(); // (m_fr_menu->GetDefaultSize());
  m_fr_menu->MapWindow();
}

void* EventDispUI::FuncNewEventCheck(void* arg)
{
  EventDispUI* ui = (EventDispUI*)arg;
  ui->ExecNewEventCheck();
}

void EventDispUI::ExecNewEventCheck()
{
  while (true) {
    if (m_auto_mode) {
      cout << "AutoMode()" << endl;
      if (FindNewRuns()) {
        cout << "  New run." << endl;
        OpenRunFile(m_list_run.back());
      }
      int n_now = FetchNumEvents(m_run);
      if (n_now > m_i_evt) {
        cout << "  New event." << endl;
        MoveEvent(n_now);
      }
    }
    sleep(15);
  }
}

