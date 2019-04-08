/// OnlMonMainDaq.C
#include <iomanip>
#include <TH1D.h>
#include <TSocket.h>
#include <TClass.h>
#include <TMessage.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQSpill.h>
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
  cout << "SQRun: \n"
       << "  " << run_header->get_run_id()
       << "  " << run_header->get_spill_count()
       << endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::process_event(PHCompositeNode* topNode)
{
  SQSpillMap*     spill_map = findNode::getClass<SQSpillMap >(topNode, "SQSpillMap");
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!spill_map || !event_header || !hit_vec || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  static int spill_id_pre = -1;
  if (event_header->get_spill_id() != spill_id_pre) {
    spill_id_pre = event_header->get_spill_id();
    SQSpill* spi = spill_map->get(spill_id_pre);
    //PrintSpill(spi);
    h1_tgt->Fill(spi->get_target_pos());
  }

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
  h1_tgt      = FindMonHist("h1_tgt");
  h1_evt_qual = FindMonHist("h1_evt_qual");

  pad_title->cd();
  TPaveText* title = new TPaveText(.02, .02, .98, .98);
  title->AddText(Name().c_str());
  title->Draw();

  pad_main->cd();
  pad_main->SetGrid();
  pad_main->Divide(1, 2);

  pad_main->cd(1);
  if (h1_evt_qual->Integral() > 100) gPad->SetLogy();
  h1_evt_qual->Draw();

  pad_main->cd(2);
  h1_tgt->Draw();

  pad_msg->cd();
  TPaveText* msg = new TPaveText(.02, .02, .98, .98);
  msg->SetFillColor(kGreen);
  msg->AddText("Always Okay ;^D");
  msg->Draw();
  
  return 0;
}


void OnlMonMainDaq::PrintSpill(SQSpill* spi)
{
  cout << "SQSpill:  "
       << "  " << spi->get_spill_id    ()
       << "  " << spi->get_run_id      ()
       << "  " << spi->get_target_pos  ()
       << "  " << spi->get_bos_coda_id ()
       << "  " << spi->get_bos_vme_time()
       << "  " << spi->get_eos_coda_id ()
       << "  " << spi->get_eos_vme_time()
       << "  \nBOS Scaler:  " << spi->get_bos_scaler_list()->size() << "\n";
  for (SQStringMap::ConstIter it = spi->get_bos_scaler_list()->begin(); it != spi->get_bos_scaler_list()->end(); it++) {
    string name = it->first;
    SQScaler* sca = dynamic_cast<SQScaler*>(it->second);
    cout << "    " << setw(20) << name << " " << setw(10) << sca->get_count() << "\n";
  }
  cout << "  EOS Scaler:  " << spi->get_eos_scaler_list()->size() << "\n";
  for (SQStringMap::ConstIter it = spi->get_eos_scaler_list()->begin(); it != spi->get_eos_scaler_list()->end(); it++) {
    string name = it->first;
    SQScaler* sca = dynamic_cast<SQScaler*>(it->second);
    cout << "  " << setw(20) << name << " " << setw(10) << sca->get_count() << "\n";
  }
  cout << "  Slow Control  " << spi->get_slow_cont_list()->size() << ":\n";
  for (SQStringMap::ConstIter it = spi->get_slow_cont_list()->begin(); it != spi->get_slow_cont_list()->end(); it++) {
    string name = it->first;
    SQSlowCont* slo = dynamic_cast<SQSlowCont*>(it->second);
    cout << "  " << setw(20) << name << " " << slo->get_time_stamp() << " " << setw(20) << slo->get_value() << " " << slo->get_type() << "\n";
  }
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
