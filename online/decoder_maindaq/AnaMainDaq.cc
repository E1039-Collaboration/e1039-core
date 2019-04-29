/*
 * AnaMainDaq.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#include <iomanip>
#include <TH1D.h>
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
#include <onlmonserver/OnlMonServer.h>
#include "AnaMainDaq.h"
using namespace std;

AnaMainDaq::AnaMainDaq(const std::string& name) : SubsysReco(name)
{
  ;
}

int AnaMainDaq::Init(PHCompositeNode* topNode)
{
  h1_evt_qual = new TH1D("h1_evt_qual", ";Event-quality bit;N of events", 33, -0.5, 32.5);

  Fun4AllHistoManager* hm = new Fun4AllHistoManager("MainDaq");
  OnlMonServer::instance()->registerHistoManager(hm);
  hm->registerHisto(h1_evt_qual);

  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaMainDaq::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  cout << "SQRun: \n"
       << "  " << run_header->get_run_id()
       << "  " << run_header->get_spill_count()
       << endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaMainDaq::process_event(PHCompositeNode* topNode)
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
    PrintSpill(spi);
  }

  static int n_evt_print = 0;
  if (n_evt_print++ < 3) PrintEvent(event_header, hit_vec, trig_hit_vec);

  int dq_evt = event_header->get_data_quality();
  if (dq_evt == 0) h1_evt_qual->Fill(32);
  for (int bit = 0; bit < 32; bit++) {
    if ((dq_evt > bit) & 0x1) h1_evt_qual->Fill(bit);
  }

  
  return Fun4AllReturnCodes::EVENT_OK;
}

int AnaMainDaq::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void AnaMainDaq::PrintSpill(SQSpill* spi)
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

void AnaMainDaq::PrintEvent(SQEvent* evt, SQHitVector* v_hit, SQHitVector* v_trig_hit)
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
