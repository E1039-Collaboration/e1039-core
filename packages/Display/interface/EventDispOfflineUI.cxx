#include <iostream>
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
#include "EventDispOfflineUI.h"
using namespace std;

EventDispOfflineUI::EventDispOfflineUI()
  : m_fn_dst("")
{
  m_online_mode = false;
  m_auto_mode   = false;
}

int EventDispOfflineUI::FetchNumEvents(const std::string fn_dst)
{
  int ret = -1;
  TFile* file = new TFile(fn_dst.c_str());
  if (file->IsOpen()) {
    TTree* tree = (TTree*)file->Get("T");
    if (tree) ret = tree->GetEntries();
  }
  delete file;
  return ret;
}

//void EventDispOfflineUI::NextEvent()
//{
//  if (m_online_mode && m_auto_mode) {
//    cout << "NextEvent(): Ignored in auto mode." << endl;
//    return;
//  }
//  if (m_i_evt >= m_n_evt) {
//    cout << "NextEvent(): No next event." << endl;
//    return;
//  }
//  Fun4AllServer* se = Fun4AllServer::instance();
//  m_i_evt++;
//  cout << "Next: " << m_i_evt << " / " << m_n_evt << endl;
//  UpdateLabels();
//  if (se->run(1, true) != 0) {
//    cout << "NextEvent() failed.  No event remains?" << endl;
//  }
//}

void EventDispOfflineUI::PrevEvent()
{
  int i_new = m_i_evt - 1;
  cout << "Prev: " << i_new << " / " << m_n_evt << endl;

  Fun4AllServer* se = Fun4AllServer::instance();
  Fun4AllInputManager *in = se->getInputManager("DSTIN");
  if (in->fileopen(m_fn_dst) != 0) {
    cout << "PrevEvent(): Cannot reopen DST." << endl;
    return;
  }

  m_i_evt = i_new;
  UpdateLabels();
  cout << "m_i_evt = " << m_i_evt << endl;
  if (m_i_evt > 1) se->skip(m_i_evt - 1); // First move to the previous-to-previous event.
  if (se->run(1, true) != 0) { // Then read the previous event.
    cout << "PrevEvent() failed." << endl;
  }
}

//void EventDispOfflineUI::MoveEvent(const int i_evt)
//{
//  if (i_evt > m_n_evt) {
//    OpenRunFile(m_run, m_spill);
//    if (i_evt > m_n_evt) {
//      cout << "Unexpected!!" << endl;
//      return;
//    }
//  }
//  int i_move = i_evt - m_i_evt;
//  m_i_evt = i_evt;
//  UpdateLabels();
//  Fun4AllServer* se = Fun4AllServer::instance();
//  if (i_move > 1) se->skip(i_move - 1); // First move to the previous event.
//  if (se->run(1, true) != 0) { // Then read the previous event.
//    cout << "MoveEvent() failed." << endl;
//  }
//}
//
//void EventDispOfflineUI::ReqEvtID()
//{
//  int num = (int)m_ne_evt_id->GetNumberEntry()->GetIntNumber();
//  cout << "ReqEvtID: " << num << endl;
//  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
//  filter->set_event_id_req(num);
//}
//
//void EventDispOfflineUI::ReqTrig()
//{
//  int num = (int)m_ne_trig->GetNumberEntry()->GetIntNumber();
//  cout << "ReqTrig: " << num << endl;
//  EvtFilter* filter = (EvtFilter*)Fun4AllServer::instance()->getSubsysReco("EvtFilter");
//  filter->set_trigger_req(num);
//}
//
//void EventDispOfflineUI::ViewTop()
//{
//  cout << "Top View" << endl;
//  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
//  disp->set_view_top();
//}
//
//void EventDispOfflineUI::ViewSide()
//{
//  cout << "Side View" << endl;
//  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
//  disp->set_view_side();
//}
//
//void EventDispOfflineUI::View3D()
//{
//  cout << "3D View" << endl;
//  PHEventDisplay* disp = (PHEventDisplay*)Fun4AllServer::instance()->getSubsysReco("PHEventDisplay");
//  disp->set_view_3d();
//}
//
//void EventDispOfflineUI::UpdateLabels()
//{
//  ostringstream oss;
//  oss << (m_online_mode ? "Online" : "Offline") << " Mode";
//  m_lbl_mode->SetText(oss.str().c_str());
//
//  oss.str("");
//  oss << "Run " << m_run;
//  m_lbl_run->SetText(oss.str().c_str());
//
//  oss.str("");
//  oss << "Event " << m_i_evt << " / " << m_n_evt;
//  m_lbl_n_evt->SetText(oss.str().c_str());
//}

void EventDispOfflineUI::Run(const int run_id, const std::string fn_dst)
{
  cout << "EventDispOfflineUI::Run(): " << run_id << " " << fn_dst << endl;
  BuildInterface();
  m_run = run_id;
  m_fn_dst = fn_dst;
  m_n_evt = FetchNumEvents(fn_dst);
  m_i_evt = 0;
  UpdateLabels();
  Fun4AllInputManager *in = Fun4AllServer::instance()->getInputManager("DSTIN");
  in->fileopen(fn_dst);
  MoveEvent(1); // Need process one event here, before calling "FuncNewEventCheck"
}

//void EventDispOfflineUI::UpdateInterface()
//{
//  if (m_online_mode) {
//    m_fr_menu->ShowFrame(m_fr_evt_mode);
//    m_fr_menu->HideFrame(m_fr_evt_sel);
//    if (m_auto_mode) {
//      m_fr_menu->HideFrame(m_fr_evt_next);
//      m_fr_menu->HideFrame(m_fr_evt_prev);
//    } else {
//      m_fr_menu->ShowFrame(m_fr_evt_next);
//      m_fr_menu->ShowFrame(m_fr_evt_prev);
//    }
//  } else {
//    m_fr_menu->HideFrame(m_fr_evt_mode);
//    m_fr_menu->ShowFrame(m_fr_evt_sel);
//    m_fr_menu->ShowFrame(m_fr_evt_next);
//    m_fr_menu->ShowFrame(m_fr_evt_prev); // HideFrame(m_fr_evt_prev);
//  }
//  m_fr_menu->Resize(); // (m_fr_menu->GetDefaultSize());
//  m_fr_menu->MapWindow();
//}
