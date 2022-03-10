/// OnlMonQie.C
#include <sstream>
#include <iomanip>
#include <TH1D.h>
#include <TH2D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHardEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <UtilAna/UtilBeam.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonQie.h"
using namespace std;

OnlMonQie::OnlMonQie()
{
  Name("OnlMonQie");
  Title("QIE");
  NumCanvases(2);
}

int OnlMonQie::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonQie::InitRunOnlMon(PHCompositeNode* topNode)
{
  h1_evt_status = new TH1D("h1_evt_status", "Event Status;Status;N of events", 7, 0.5, 7.5);
  h1_evt_status->GetXaxis()->SetBinLabel(1, "All");
  h1_evt_status->GetXaxis()->SetBinLabel(2, " ");
  h1_evt_status->GetXaxis()->SetBinLabel(3, "No data");
  h1_evt_status->GetXaxis()->SetBinLabel(4, "Extra data");
  h1_evt_status->GetXaxis()->SetBinLabel(5, "No RF ID");
  h1_evt_status->GetXaxis()->SetBinLabel(6, "RF+-13 = 0");
  h1_evt_status->GetXaxis()->SetBinLabel(7, "RF+-08 = 0");

  //h1_trig_cnt = new TH1D("h1_trig_cnt", ";Trigger counts;N of events", 100, -0.5, 4999.5);
  //h2_presum   = new TH2D("h2_presum" , ";Presum;Index", 100, -0.5, 4999.5,  N_PRESUM, -0.5, N_PRESUM-0.5);
  h2_turn_rf = new TH2D("h2_turn_rf", "Turn+RF IDs of events;Turn ID;RF ID", 369, 0.5, 369000.5,  588, 0.5, 588.5);

  int num_inte;
  double* list_inte;
  UtilBeam::ListOfRfValues(num_inte, list_inte);
  h2_inte_rf = new TH2D("h2_inte_rf", "RF intensity;RF intensity;RF+nn", num_inte-1, list_inte, N_RF_INTE, -N_RF_INTE/2.0, N_RF_INTE/2.0);

  RegisterHist(h1_evt_status);
  //RegisterHist(h1_trig_cnt);
  //RegisterHist(h2_presum);
  RegisterHist(h2_turn_rf);
  RegisterHist(h2_inte_rf);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonQie::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQRun*        run = findNode::getClass<SQRun      >(topNode, "SQRun");
  SQEvent*      evt = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHardEvent* hevt = findNode::getClass<SQHardEvent>(topNode, "SQHardEvent");
  if (! run || ! evt || ! hevt) return Fun4AllReturnCodes::ABORTEVENT;

  h1_evt_status->Fill(ALL);

  int n_brd = hevt->get_n_board_qie();
  if (n_brd == 0) {
    h1_evt_status->Fill(NO_DATA);
    return Fun4AllReturnCodes::EVENT_OK;
  } else if (n_brd > 1) {
    h1_evt_status->Fill(EXTRA_DATA);
    return Fun4AllReturnCodes::EVENT_OK;
  }

  //h1_trig_cnt->Fill(evt->get_qie_trigger_count());
  //for (int ii = 0; ii < N_PRESUM; ii++) {
  //  h2_presum->Fill(evt->get_qie_presum(ii), ii);
  //}
  int turn_id = evt->get_qie_turn_id();
  int   rf_id = evt->get_qie_rf_id();
  h2_turn_rf->Fill(turn_id, rf_id);
  if (turn_id <= 0 || rf_id <= 0) h1_evt_status->Fill(TURN_RF_ID_ZERO);

  bool found_zero_pm13 = false;
  bool found_zero_pm08 = false;
  for (int ii = -N_RF_INTE/2; ii <= N_RF_INTE/2; ii++) {
    int inte = evt->get_qie_rf_intensity(ii);
    h2_inte_rf->Fill(inte, ii);
    if (inte == 0) {
      if (abs(ii) <= 13) found_zero_pm13 = true;
      if (abs(ii) <=  8) found_zero_pm08 = true;
    }
  }
  if (found_zero_pm13) h1_evt_status->Fill(ZERO_PM13);
  if (found_zero_pm08) h1_evt_status->Fill(ZERO_PM08);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonQie::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonQie::FindAllMonHist()
{
  h1_evt_status = FindMonHist("h1_evt_status");
  //h1_trig_cnt   = FindMonHist("h1_trig_cnt");
  //h2_presum     = (TH2*)FindMonHist("h2_presum");
  h2_turn_rf    = (TH2*)FindMonHist("h2_turn_rf");
  h2_inte_rf    = (TH2*)FindMonHist("h2_inte_rf");
  return (h1_evt_status && h2_turn_rf && h2_inte_rf  ?  0  :  1);
}

int OnlMonQie::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1, 2);

  TVirtualPad* pad01 = pad0->cd(1);
  pad01->SetGrid();
  h1_evt_status->Draw();

  TVirtualPad* pad02 = pad0->cd(2);
  pad02->SetGrid();
  h2_turn_rf->Draw("colz");

  double n_evt_all = h1_evt_status->GetBinContent(ALL); // bin = 1
  // "bin = 2" is skipped since it means OK.
  for (int bin = 3; bin <= h1_evt_status->GetNbinsX(); bin++) {
    double rate = h1_evt_status->GetBinContent(bin) / n_evt_all;
    if (rate > 0.01) {
      ostringstream oss;
      oss << "'" << h1_evt_status->GetXaxis()->GetBinLabel(bin) << "' in " << (int)(100*rate) << "% of events.";
      can0->AddMessage(oss.str());
      can0->SetWorseStatus(OnlMonCanvas::WARN);
    }
  }
  //can0->SetStatus(OnlMonCanvas::OK);

  h2_inte_rf->GetXaxis()->SetRangeUser(0.0, 2000.0);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1, 2);

  TVirtualPad* pad11 = pad1->cd(1);
  pad11->SetGrid();
  pad11->SetLogx();
  h2_inte_rf->Draw("colz");

  TVirtualPad* pad12 = pad1->cd(2);
  pad12->SetGrid();
  pad12->SetLogx();
  int bin = h2_inte_rf->GetYaxis()->FindBin(0.0); // RF+00
  TH1* h1_p01 = h2_inte_rf->ProjectionX("h1_p01", bin+1, bin+1);
  TH1* h1_p00 = h2_inte_rf->ProjectionX("h1_p00", bin  , bin  );
  TH1* h1_m01 = h2_inte_rf->ProjectionX("h1_m01", bin-1, bin-1);
  h1_p00->SetLineWidth(2);
  h1_p00->SetLineColor(kRed);
  h1_p01->SetLineColor(kBlue);
  h1_m01->SetLineColor(kBlack);
  THStack* hs = new THStack("hs", "RF intensity @ RF+nn;RF intensity;N of events");
  hs->Add(h1_p00, "l");
  hs->Add(h1_p01, "l");
  hs->Add(h1_m01, "l");
  hs->Draw("nostack");
  TLegend* leg = new TLegend(0.8, 0.8, 0.99, 0.99);
  leg->AddEntry(h1_p01, "RF+01", "l");
  leg->AddEntry(h1_p00, "RF+00", "l");
  leg->AddEntry(h1_m01, "RF-01", "l");
  leg->Draw();

  return 0;
}
