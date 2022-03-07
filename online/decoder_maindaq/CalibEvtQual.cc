#include <interface_main/SQEvent.h>
#include <interface_main/SQHardEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "CalibEvtQual.h"
using namespace std;

CalibEvtQual::CalibEvtQual(const std::string& name) : SubsysReco(name)
{
  ;
}

int CalibEvtQual::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibEvtQual::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibEvtQual::process_event(PHCompositeNode* topNode)
{
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! event) return Fun4AllReturnCodes::ABORTEVENT;

  SQHardEvent* hard_evt = findNode::getClass<SQHardEvent>(topNode, "SQHardEvent");
  if (! hard_evt) return Fun4AllReturnCodes::ABORTEVENT;

  int run_id = event->get_run_id();
  int qual   = event->get_data_quality();

  int n_tdc; // expected number of tdc info
  if (run_id >= 28664) n_tdc = 101; // seen in run 28700
  else                 n_tdc = 108; // seen in run 28000
  // This run range was confirmed by checking n_tdc in the following runs;
  //  * 101 in runs 28680, 28670, 28665, 28664
  //  * 108 in runs 28500, 28600, 28650, 28660, 28661
  // Note that runs 28662 & 28663 contain no event.
  // Is the change of n_tdc related to elog #17385??
  // https://e906-gat6.fnal.gov:8080/SeaQuest/17385
  if      (hard_evt->get_n_board_taiwan() != n_tdc) qual |= ERR_N_TDC;
  if      (hard_evt->get_n_board_v1495      () < 2) qual |= ERR_N_V1495_0;
  else if (hard_evt->get_n_board_v1495      () > 2) qual |= ERR_N_V1495_2;
  if      (hard_evt->get_n_board_trig_bit   () < 1) qual |= ERR_N_TRIGB_0;
  else if (hard_evt->get_n_board_trig_bit   () > 1) qual |= ERR_N_TRIGB_2;
  if      (hard_evt->get_n_board_trig_count () < 1) qual |= ERR_N_TRIGC_0;
  else if (hard_evt->get_n_board_trig_count () > 1) qual |= ERR_N_TRIGC_2;
  if      (hard_evt->get_n_board_qie        () < 1) qual |= ERR_N_QIE_0;
  else if (hard_evt->get_n_board_qie        () > 1) qual |= ERR_N_QIE_2;

  event->set_data_quality(qual);
  //if (qual != 0) {
  //  cout << "  N! " << evt_id << " | " << ed->n_qie << " " << ed->n_v1495 << " " << ed->n_tdc << " " << ed->n_trig_b << " " << ed->n_trig_c << endl;
  //  }
  //}
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibEvtQual::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void CalibEvtQual::PrintEvent(SQEvent* evt, SQHardEvent* hevt)
{
  cout << "SQEvent:  "
       << "  " <<  evt->get_run_id       ()
       << "  " <<  evt->get_spill_id     ()
       << "  " <<  evt->get_event_id     ()
       << "  " <<  evt->get_data_quality ()
       << "  " << hevt->get_coda_event_id()
       << "  " << hevt->get_vme_time     ()
       << "\n";
}
