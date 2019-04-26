/// OnlMonMainDaq.C
#include <iomanip>
#include <TH1D.h>
#include <TSocket.h>
#include <TClass.h>
#include <TMessage.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQStringMap.h>
#include <interface_main/SQScaler.h>
#include <interface_main/SQSlowCont.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "OnlMonServer.h"
#include "OnlMonMainDaq.h"
using namespace std;

OnlMonMainDaq::OnlMonMainDaq(const std::string& name) : OnlMonClient(name)
{
  ;
}

int OnlMonMainDaq::Init(PHCompositeNode* topNode)
{
  h1_tgt      = new TH1D("h1_tgt", ";Target position; N of spills", 9, -0.5, 8.5);
  h1_evt_qual = new TH1D("h1_evt_qual", ";Event-quality bit;N of events", 33, -0.5, 32.5);

  Fun4AllHistoManager* hm = new Fun4AllHistoManager(Name());
  OnlMonServer::instance()->registerHistoManager(hm);
  hm->registerHisto(h1_tgt);
  hm->registerHisto(h1_evt_qual);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  cout << "SQRun: " << run_header->get_run_id() << " " << run_header->get_spill_count() << endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::process_event(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!event_header || !hit_vec || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  //static int n_evt_print = 0;
  //if (n_evt_print++ < 3) PrintEvent(event_header, hit_vec, trig_hit_vec);

  int dq_evt = event_header->get_data_quality();
  if (dq_evt == 0) h1_evt_qual->Fill(32);
  for (int bit = 0; bit < 32; bit++) {
    if ((dq_evt >> bit) & 0x1) h1_evt_qual->Fill(bit);
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::DrawMonitor()
{
  //h1_tgt      = FindMonHist("h1_tgt");
  //h1_evt_qual = FindMonHist("h1_evt_qual");
  h1_tgt      = (TH1*)FindMonObj("h1_tgt");
  h1_evt_qual = (TH1*)FindMonObj("h1_evt_qual");

  pad_main->cd();
  pad_main->SetGrid();
  pad_main->Divide(1, 2);

  pad_main->cd(1);
  if (h1_evt_qual->Integral() > 100) gPad->SetLogy();
  h1_evt_qual->Draw();

  pad_main->cd(2);
  h1_tgt->Draw();

  AddMessage("Always Okay ;^D");
  SetStatus(OK);

  return 0;
}

void OnlMonMainDaq::PrintEvent(SQEvent* evt, SQHitVector* v_hit, SQHitVector* v_trig_hit)
{
  cout << "SQEvent:  "
       << "  " << evt->get_run_id       ()
       << "  " << evt->get_spill_id     ()
       << "  " << evt->get_event_id     ()
       << "  " << evt->get_coda_event_id()
       << "  " << evt->get_data_quality ()
       << "  " << evt->get_vme_time     ()
       << "\n";

  cout << "SQHitVector (Taiwan TDC):  " << v_hit->size() << "\n";
  for (SQHitVector::ConstIter it = v_hit->begin(); it != v_hit->end(); it++) {
    cout << "    " << (*it)->get_hit_id() << " " << setw(2) << (*it)->get_detector_id() << " " << setw(3) << (*it)->get_element_id() << " " << (*it)->get_tdc_time() << "\n";
  }

  cout << "SQHitVector (v1495 TDC):  " << v_trig_hit->size() << "\n";
  for (SQHitVector::ConstIter it = v_trig_hit->begin(); it != v_trig_hit->end(); it++) {
    cout << "    " << (*it)->get_hit_id() << " " << setw(2) << (*it)->get_detector_id() << " " << setw(3) << (*it)->get_element_id() << " " << (*it)->get_tdc_time() << "\n";
  }
}
