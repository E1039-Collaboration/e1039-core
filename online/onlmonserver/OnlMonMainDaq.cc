/// OnlMonMainDaq.C
#include <sstream>
#include <iomanip>
#include <TH1D.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonParam.h"
#include "OnlMonMainDaq.h"
using namespace std;

OnlMonMainDaq::OnlMonMainDaq()
{
  Name("OnlMonMainDaq");
  Title("Main DAQ");
  NumCanvases(1);
}

int OnlMonMainDaq::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::InitRunOnlMon(PHCompositeNode* topNode)
{
  h1_trig = new TH1D("h1_trig", "Trigger Status;Trigger;N of events", 10, 0.5, 10.5);
  h1_n_taiwan = new TH1D("h1_n_taiwan", "Taiwan TDC Status;N of boards/event;N of events", 200, -0.5, 199.5);
  h1_evt_qual = new TH1D("h1_evt_qual", "Event Status;Event-quality bit;N of events", 33, -0.5, 32.5);
  h1_flag_v1495 = new TH1D("h1_flag_v1495", "V1495 Status;v1495 status bit; N of v1495 events", 5, -0.5, 4.5);
  h1_cnt = new TH1D("h1_cnt", ";Type;Count", 15, 0.5, 15.5);

  h1_trig->GetXaxis()->SetBinLabel( 1, "FPGA1");
  h1_trig->GetXaxis()->SetBinLabel( 2, "FPGA2");
  h1_trig->GetXaxis()->SetBinLabel( 3, "FPGA3");
  h1_trig->GetXaxis()->SetBinLabel( 4, "FPGA4");
  h1_trig->GetXaxis()->SetBinLabel( 5, "FPGA5");
  h1_trig->GetXaxis()->SetBinLabel( 6, "NIM1");
  h1_trig->GetXaxis()->SetBinLabel( 7, "NIM2");
  h1_trig->GetXaxis()->SetBinLabel( 8, "NIM3");
  h1_trig->GetXaxis()->SetBinLabel( 9, "NIM4");
  h1_trig->GetXaxis()->SetBinLabel(10, "NIM5");

  ostringstream oss;
  h1_evt_qual->GetXaxis()->SetBinLabel(1, "OK");
  for (int ii = 1; ii <= 32; ii++) {
    oss.str("");  oss << ii;
    h1_evt_qual->GetXaxis()->SetBinLabel(ii+1, oss.str().c_str());
  }

  h1_flag_v1495->GetXaxis()->SetBinLabel(1, "OK");
  h1_flag_v1495->GetXaxis()->SetBinLabel(2, "d1ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(3, "d2ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(4, "d3ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(5, "Other");

  RegisterHist(h1_trig);
  RegisterHist(h1_n_taiwan);
  RegisterHist(h1_evt_qual);
  RegisterHist(h1_flag_v1495);
  RegisterHist(h1_cnt, OnlMonClient::MODE_UPDATE);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQRun*   run   = findNode::getClass<SQRun  >(topNode, "SQRun");
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! run || ! event) return Fun4AllReturnCodes::ABORTEVENT;

  if (event->get_trigger(SQEvent::MATRIX1)) h1_trig->Fill( 1);
  if (event->get_trigger(SQEvent::MATRIX2)) h1_trig->Fill( 2);
  if (event->get_trigger(SQEvent::MATRIX3)) h1_trig->Fill( 3);
  if (event->get_trigger(SQEvent::MATRIX4)) h1_trig->Fill( 4);
  if (event->get_trigger(SQEvent::MATRIX5)) h1_trig->Fill( 5);
  if (event->get_trigger(SQEvent::NIM1   )) h1_trig->Fill( 6);
  if (event->get_trigger(SQEvent::NIM2   )) h1_trig->Fill( 7);
  if (event->get_trigger(SQEvent::NIM3   )) h1_trig->Fill( 8);
  if (event->get_trigger(SQEvent::NIM4   )) h1_trig->Fill( 9);
  if (event->get_trigger(SQEvent::NIM5   )) h1_trig->Fill(10);

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

  h1_n_taiwan->Fill(event->get_n_board_taiwan());

  int dq = event->get_data_quality();
  if (dq == 0) h1_evt_qual->Fill(0);
  for (int bit = 0; bit < 32; bit++) {
    if ((dq >> bit) & 0x1) h1_evt_qual->Fill(bit + 1);
  }

  int v1495 = event->get_flag_v1495();
  if (v1495 == 0) h1_flag_v1495->Fill(0);
  for (int bit = 0; bit < 4; bit++) {
    if ((v1495 >> bit) & 0x1) h1_flag_v1495->Fill(bit + 1);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::FindAllMonHist()
{
  h1_trig       = FindMonHist("h1_trig");
  h1_n_taiwan   = FindMonHist("h1_n_taiwan");
  h1_evt_qual   = FindMonHist("h1_evt_qual");
  h1_flag_v1495 = FindMonHist("h1_flag_v1495");
  h1_cnt        = FindMonHist("h1_cnt");
  return (h1_trig && h1_evt_qual && h1_flag_v1495 && h1_cnt  ?  0  :  1);
}

int OnlMonMainDaq::DrawMonitor()
{
  OnlMonParam param(this);
  int n_ttdc = param.GetIntParam("N_TAIWAN_TDC");

  UtilHist::AutoSetRange(h1_n_taiwan);

  OnlMonCanvas* can = GetCanvas();
  can->SetStatus(OnlMonCanvas::OK);

  TPad* pad = can->GetMainPad();
  pad->Divide(1, 3);

  pad->cd(1);
  TPad* pad11 = new TPad("pad11", "", 0.0, 0.0, 0.6, 1.0);
  TPad* pad12 = new TPad("pad12", "", 0.6, 0.0, 1.0, 1.0);
  pad11->Draw();
  pad12->Draw();

  pad11->cd();
  pad11->SetGrid();
  h1_trig->Draw();

  pad12->cd();
  pad12->SetGrid();
  h1_flag_v1495->Draw();

  pad->cd(2);
  TPad* pad21 = new TPad("pad21", "", 0.0, 0.0, 0.7, 1.0);
  TPad* pad22 = new TPad("pad22", "", 0.7, 0.0, 1.0, 1.0);
  pad21->Draw();
  pad22->Draw();

  pad21->cd();
  pad21->SetGrid();
  h1_evt_qual->Draw();

  pad22->cd();
  pad22->SetGrid();
  h1_n_taiwan->Draw();
  double n_ttdc_mean = h1_n_taiwan->GetMean();
  if (fabs(n_ttdc_mean - n_ttdc) > 0.1) {
    can->SetWorseStatus(OnlMonCanvas::ERROR);
    can->AddMessage(TString::Format("N of Taiwan TDCs = %.1f, not %d.", n_ttdc_mean, n_ttdc).Data());
  }

  pad->cd(3);
  TPaveText* pate = new TPaveText(.02, .02, .98, .98);
  ostringstream oss;
  oss << "N of spill-counter events = " << h1_cnt->GetBinContent(1);
  pate->AddText(oss.str().c_str());
  oss.str("");
  oss << "N of triggered events = " << h1_cnt->GetBinContent(2) << " (all), " << h1_cnt->GetBinContent(3) << " (decoded)";
  pate->AddText(oss.str().c_str());

  double n_all = h1_cnt->GetBinContent(4);
  double n_bad = h1_cnt->GetBinContent(5);
  oss.str("");
  oss << "N of Coda physics events = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());

  n_all = h1_cnt->GetBinContent(6);
  n_bad = h1_cnt->GetBinContent(7);
  oss.str("");
  oss << "N of Coda flush events = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());
  if (n_bad / n_all > 0.05) {
    can->SetWorseStatus(OnlMonCanvas::ERROR);
    can->AddMessage("Errors in >5% of Coda flush events.");
  } else if (n_bad / n_all > 0.01) {
    can->SetWorseStatus(OnlMonCanvas::WARN);
    can->AddMessage("Errors in >1% of Coda flush events.");
  }

  n_all = h1_cnt->GetBinContent(8);
  n_bad = h1_cnt->GetBinContent(10);
  oss.str("");
  oss << "N of Taiwan TDC hits = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());
  if (n_bad / n_all > 0.05) {
    can->SetWorseStatus(OnlMonCanvas::ERROR);
    can->AddMessage("Errors in >5% of Taiwan TDC hits.");
  } else if (n_bad / n_all > 0.01) {
    can->SetWorseStatus(OnlMonCanvas::WARN);
    can->AddMessage("Errors in >1% of Taiwan TDC hits.");
  }

  n_all = h1_cnt->GetBinContent(9);
  n_bad = h1_cnt->GetBinContent(11);
  oss.str("");
  oss << "N of v1495 TDC hits = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());
  if (n_bad / n_all > 0.05) {
    can->SetWorseStatus(OnlMonCanvas::ERROR);
    can->AddMessage("Errors in >5% of v1495 TDC hits.");
  } else if (n_bad / n_all > 0.01) {
    can->SetWorseStatus(OnlMonCanvas::WARN);
    can->AddMessage("Errors in >1% of v1495 TDC hits.");
  }

  n_all         = h1_cnt->GetBinContent(12);
  double n_d1ad = h1_cnt->GetBinContent(13);
  double n_d2ad = h1_cnt->GetBinContent(14);
  double n_d3ad = h1_cnt->GetBinContent(15);
  oss.str("");
  oss << "N of v1495 events = " << n_all << " (all), " << n_d1ad << " (d1ad), " << n_d2ad << " (d2ad), " << n_d3ad << " (d3ad)";
  pate->AddText(oss.str().c_str());
  pate->Draw();
  if ((n_d1ad + n_d2ad + n_d3ad) / n_all > 0.05) {
    can->SetWorseStatus(OnlMonCanvas::ERROR);
    can->AddMessage("Errors in >5% of v1495 events.");
  } else if ((n_d1ad + n_d2ad + n_d3ad) / n_all > 0.01) {
    can->SetWorseStatus(OnlMonCanvas::WARN);
    can->AddMessage("Errors in >1% of v1495 events.");
  }

  return 0;
}
