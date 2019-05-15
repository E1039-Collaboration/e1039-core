/// OnlMonMainDaq.C
#include <iomanip>
#include <TH1D.h>
#include <TSocket.h>
#include <TClass.h>
#include <TMessage.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
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
  SetNumCanvases(1);
}

int OnlMonMainDaq::Init(PHCompositeNode* topNode)
{
  h1_evt_qual = new TH1D("h1_evt_qual", ";Event-quality bit;N of events", 33, -0.5, 32.5);
  h1_flag_v1495 = new TH1D("h1_flag_v1495", ";v1495 status flag; N of v1495 events", 4, -0.5, 3.5);
  h1_cnt = new TH1D("h1_cnt", ";Type;Count", 15, 0.5, 15.5);

  Fun4AllHistoManager* hm = new Fun4AllHistoManager(Name());
  OnlMonServer::instance()->registerHistoManager(hm);
  hm->registerHisto(h1_evt_qual);
  hm->registerHisto(h1_flag_v1495);
  hm->registerHisto(h1_cnt);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::process_event(PHCompositeNode* topNode)
{
  SQRun*   run   = findNode::getClass<SQRun  >(topNode, "SQRun");
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! run || ! event) return Fun4AllReturnCodes::ABORTEVENT;

  h1_cnt->SetBinContent( 1, run->get_n_spill        ());
  h1_cnt->SetBinContent( 2, run->get_n_evt_all      ());
  h1_cnt->SetBinContent( 3, run->get_n_evt_dec      ());
  h1_cnt->SetBinContent( 4, run->get_n_phys_evt     ());
  h1_cnt->SetBinContent( 5, run->get_n_phys_evt_bad ());
  h1_cnt->SetBinContent( 6, run->get_n_flush_evt    ());
  h1_cnt->SetBinContent( 7, run->get_n_flush_evt_bad());
  h1_cnt->SetBinContent( 8, run->get_n_hit          ());
  h1_cnt->SetBinContent( 9, run->get_n_t_hit        ());
  h1_cnt->SetBinContent(10, run->get_n_hit_bad      ());
  h1_cnt->SetBinContent(11, run->get_n_t_hit_bad    ());
  h1_cnt->SetBinContent(12, run->get_n_v1495        ());
  h1_cnt->SetBinContent(13, run->get_n_v1495_d1ad   ());
  h1_cnt->SetBinContent(14, run->get_n_v1495_d2ad   ());
  h1_cnt->SetBinContent(15, run->get_n_v1495_d3ad   ());

  int dq = event->get_data_quality();
  if (dq == 0) h1_evt_qual->Fill(0);
  for (int bit = 0; bit < 32; bit++) {
    if ((dq >> bit) & 0x1) h1_evt_qual->Fill(bit + 1);
  }

  int v1495 = event->get_flag_v1495();
  if (v1495 == 0) h1_flag_v1495->Fill(0);
  for (int bit = 0; bit < 3; bit++) {
    if ((v1495 >> bit) & 0x1) h1_flag_v1495->Fill(bit + 1);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::DrawMonitor()
{
  h1_evt_qual   = (TH1*)FindMonObj("h1_evt_qual");
  h1_flag_v1495 = (TH1*)FindMonObj("h1_flag_v1495");
  h1_cnt        = (TH1*)FindMonObj("h1_cnt");

  OnlMonCanvas* can = GetCanvas();
  TPad* pad = can->GetMainPad();
  pad->SetGrid();
  pad->Divide(1, 3);

  pad->cd(1);
  if (h1_evt_qual->Integral() > 100) gPad->SetLogy();
  h1_evt_qual->Draw();

  pad->cd(2);
  if (h1_flag_v1495->Integral() > 100) gPad->SetLogy();
  h1_flag_v1495->Draw();

  pad->cd(3);
  TPaveText* pate = new TPaveText(.02, .02, .98, .98);
  ostringstream oss;
  oss << "N of spills = " << h1_cnt->GetBinContent(1);
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of triggered events = " << h1_cnt->GetBinContent(2) << " (all), " << h1_cnt->GetBinContent(3) << " (decoded)";
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of Coda physics events = " << h1_cnt->GetBinContent(4) << " (all), " << h1_cnt->GetBinContent(5) << " (bad)";
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of Coda flush events = " << h1_cnt->GetBinContent(6) << " (all), " << h1_cnt->GetBinContent(7) << " (bad)";
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of Taiwan TDC hits = " << h1_cnt->GetBinContent(8) << " (all), " << h1_cnt->GetBinContent(10) << " (bad)";
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of v1495 TDC hits = " << h1_cnt->GetBinContent(9) << " (all), " << h1_cnt->GetBinContent(11) << " (bad)";
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of v1495 events = " << h1_cnt->GetBinContent(12) << " (all), " << h1_cnt->GetBinContent(13) << " (d1ad), " << h1_cnt->GetBinContent(14) << " (d2ad), " << h1_cnt->GetBinContent(15) << " (d3ad)";
  pate->AddText(oss.str().c_str());
  pate->Draw();

  can->AddMessage("Always Okay ;^D");
  can->SetStatus(OnlMonCanvas::OK);

  return 0;
}
