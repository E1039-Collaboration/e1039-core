/// OnlMonMainDaq.C
#include <sstream>
#include <iomanip>
#include <TH1D.h>
#include <TH2D.h>
#include <THStack.h>
#include <TCanvas.h>
#include <TPaveText.h>
#include <TLegend.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQIntMap.h>
#include <interface_main/SQHardSpill.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHardEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonParam.h"
#include "OnlMonMainDaq.h"
using namespace std;

OnlMonMainDaq::OnlMonMainDaq()
  : m_spill_id_1st(0)
{
  Name("OnlMonMainDaq");
  Title("Main DAQ");
  NumCanvases(3);
}

int OnlMonMainDaq::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::InitRunOnlMon(PHCompositeNode* topNode)
{
  h1_trig = new TH1D("h1_trig", "Trigger Status;Trigger;N of events", 10, 0.5, 10.5);
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

  h1_n_taiwan = new TH1D("h1_n_taiwan", "Taiwan TDC Status;N of boards/event;N of events", 200, -0.5, 199.5);

  h1_evt_qual = new TH1D("h1_evt_qual", "Event Status;Event-quality bit;N of events", 33, -0.5, 32.5);
  ostringstream oss;
  h1_evt_qual->GetXaxis()->SetBinLabel(1, "OK");
  for (int ii = 1; ii <= 32; ii++) {
    oss.str("");  oss << ii;
    h1_evt_qual->GetXaxis()->SetBinLabel(ii+1, oss.str().c_str());
  }

  h1_flag_v1495 = new TH1D("h1_flag_v1495", "V1495 Status;v1495 status bit; N of v1495 events", 5, -0.5, 4.5);
  h1_flag_v1495->GetXaxis()->SetBinLabel(1, "OK");
  h1_flag_v1495->GetXaxis()->SetBinLabel(2, "d1ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(3, "d2ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(4, "d3ad");
  h1_flag_v1495->GetXaxis()->SetBinLabel(5, "Other");

  h1_cnt = new TH1D("h1_cnt", ";Type;Count", 15, 0.5, 15.5);

  const int N_PL = nChamberPlanes + nHodoPlanes + nPropPlanes + nDarkPhotonPlanes;
  double nhit_bin[38];
  nhit_bin[0] = 0.9; // The 1st bin for "N=0".
  nhit_bin[1] = 1.0;
  for (int ii = 0; ii < 4; ii++) {
    for (int jj = 0; jj < 9; jj++) nhit_bin[9*ii + jj + 2] = (jj + 2) * pow(10, ii);
  }
  h2_nhit_pl = new TH2D("h2_nhit_pl", ";N of hits/event;", 37, nhit_bin, N_PL+1, 0.5, N_PL+1.5);
  for (int i_pl = 1; i_pl <= N_PL; i_pl++) {
    h2_nhit_pl->GetYaxis()->SetBinLabel(i_pl, GeomSvc::instance()->getDetectorName(i_pl).c_str());
  }
  h2_nhit_pl->GetYaxis()->SetBinLabel(N_PL + 1, "All planes");

  h1_nevt_sp     = new TH1F("h1_nevt_sp"    , ";Spill;N of events"     , 1000, -0.5, 999.5);
  h1_time_input  = new TH1F("h1_time_input" , ";Spill;Wait/read input" , 1000, -0.5, 999.5);
  h1_time_deco   = new TH1F("h1_time_deco"  , ";Spill;Decode Coda file", 1000, -0.5, 999.5);
  h1_time_map    = new TH1F("h1_time_map"   , ";Spill;Map channels"    , 1000, -0.5, 999.5);
  h1_time_subsys = new TH1F("h1_time_subsys", ";Spill;Process events"  , 1000, -0.5, 999.5);
  h1_time_output = new TH1F("h1_time_output", ";Spill;Write outputs"   , 1000, -0.5, 999.5);

  RegisterHist(h1_trig);
  RegisterHist(h1_n_taiwan);
  RegisterHist(h1_evt_qual);
  RegisterHist(h1_flag_v1495);
  RegisterHist(h1_cnt, OnlMonClient::MODE_UPDATE);
  RegisterHist(h2_nhit_pl);
  RegisterHist(h1_nevt_sp);
  RegisterHist(h1_time_input);
  RegisterHist(h1_time_deco);
  RegisterHist(h1_time_map);
  RegisterHist(h1_time_subsys);
  RegisterHist(h1_time_output);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonMainDaq::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQRun*        run = findNode::getClass<SQRun      >(topNode, "SQRun");
  SQIntMap*  map_hs = findNode::getClass<SQIntMap   >(topNode, "SQHardSpillMap");
  SQEvent*      evt = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHardEvent* hevt = findNode::getClass<SQHardEvent>(topNode, "SQHardEvent");
  SQHitVector*   hv = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!run || !map_hs || !evt || !hevt || !hv) return Fun4AllReturnCodes::ABORTEVENT;

  if (evt->get_trigger(SQEvent::MATRIX1)) h1_trig->Fill( 1);
  if (evt->get_trigger(SQEvent::MATRIX2)) h1_trig->Fill( 2);
  if (evt->get_trigger(SQEvent::MATRIX3)) h1_trig->Fill( 3);
  if (evt->get_trigger(SQEvent::MATRIX4)) h1_trig->Fill( 4);
  if (evt->get_trigger(SQEvent::MATRIX5)) h1_trig->Fill( 5);
  if (evt->get_trigger(SQEvent::NIM1   )) h1_trig->Fill( 6);
  if (evt->get_trigger(SQEvent::NIM2   )) h1_trig->Fill( 7);
  if (evt->get_trigger(SQEvent::NIM3   )) h1_trig->Fill( 8);
  if (evt->get_trigger(SQEvent::NIM4   )) h1_trig->Fill( 9);
  if (evt->get_trigger(SQEvent::NIM5   )) h1_trig->Fill(10);

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

  h1_n_taiwan->Fill(hevt->get_n_board_taiwan());

  int dq = evt->get_data_quality();
  if (dq == 0) h1_evt_qual->Fill(0);
  for (int bit = 0; bit < 32; bit++) {
    if ((dq >> bit) & 0x1) h1_evt_qual->Fill(bit + 1);
  }

  int v1495 = hevt->get_flag_v1495();
  if (v1495 == 0) h1_flag_v1495->Fill(0);
  for (int bit = 0; bit < 4; bit++) {
    if ((v1495 >> bit) & 0x1) h1_flag_v1495->Fill(bit + 1);
  }

  const int N_PL = nChamberPlanes + nHodoPlanes + nPropPlanes + nDarkPhotonPlanes;
  int nhit_pl[N_PL+1]; // 0 = total, 1...N_PL = each plane
  memset(nhit_pl, 0, sizeof(nhit_pl));
  for (SQHitVector::ConstIter it = hv->begin(); it != hv->end(); it++) {
    nhit_pl[(*it)->get_detector_id()]++;
    nhit_pl[0]++;
  }
  for (int i_pl = 1; i_pl <= N_PL; i_pl++) h2_nhit_pl->Fill(nhit_pl[i_pl], i_pl);
  h2_nhit_pl->Fill(nhit_pl[0], N_PL + 1);

  int nbin_sp = h1_nevt_sp->GetNbinsX();
  int sp_id = evt->get_spill_id();
  if (m_spill_id_1st == 0) {
    h1_nevt_sp->SetBinContent(        0,     1); // UF bin to count N of histograms merged.
    h1_nevt_sp->SetBinContent(nbin_sp+1, sp_id); // OF bin to record the 1st spill ID.
    m_spill_id_1st = sp_id;
  }
  int bin_sp = sp_id - m_spill_id_1st + 1;
  if (1 <= bin_sp && bin_sp <= nbin_sp) { // Not underflow nor overflow
    h1_nevt_sp->AddBinContent(bin_sp);

    SQHardSpill* hard_sp = (SQHardSpill*)map_hs->get(sp_id);
    if (hard_sp) {
      static int sp_id_pre = 0;
      if (sp_id_pre != sp_id) { // Once at the 1st event per spill
        h1_time_input->SetBinContent(bin_sp, hard_sp->get_time_input()  / 1000);
        h1_time_deco ->SetBinContent(bin_sp, hard_sp->get_time_decode() / 1000);
        h1_time_map  ->SetBinContent(bin_sp, hard_sp->get_time_map()    / 1000);
        sp_id_pre = sp_id;
      }
      h1_time_subsys->SetBinContent(bin_sp, hard_sp->get_time_subsys() / 1000);
      h1_time_output->SetBinContent(bin_sp, hard_sp->get_time_output() / 1000);
    }
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
  h2_nhit_pl    = (TH2*)FindMonHist("h2_nhit_pl");
  h1_nevt_sp    = FindMonHist("h1_nevt_sp");
  h1_time_input  = FindMonHist("h1_time_input");
  h1_time_deco   = FindMonHist("h1_time_deco");
  h1_time_map    = FindMonHist("h1_time_map");
  h1_time_subsys = FindMonHist("h1_time_subsys");
  h1_time_output = FindMonHist("h1_time_output");
  return (h1_trig && h1_evt_qual && h1_flag_v1495 && h1_cnt && h2_nhit_pl && h1_nevt_sp &&
          h1_time_input && h1_time_deco && h1_time_map && h1_time_subsys && h1_time_output  ?  0  :  1);
}

int OnlMonMainDaq::DrawMonitor()
{
  OnlMonParam param(this);
  int n_ttdc       = param.GetIntParam("N_TAIWAN_TDC");
  int nhit_pl_warn = param.GetIntParam("N_HIT_PL_WARN");
  int nhit_pl_err  = param.GetIntParam("N_HIT_PL_ERROR");
  int nevt_sp_warn = param.GetIntParam("N_EVT_SP_WARN");
  int nevt_sp_err  = param.GetIntParam("N_EVT_SP_ERROR");

  UtilHist::AutoSetRange(h1_n_taiwan);

  OnlMonCanvas* can0 = GetCanvas(0); // ========== Canvas #0 ==========
  can0->SetStatus(OnlMonCanvas::OK);

  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1, 3);

  pad0->cd(1);
  TPad* pad011 = new TPad("pad011", "", 0.0, 0.0, 0.6, 1.0);
  TPad* pad012 = new TPad("pad012", "", 0.6, 0.0, 1.0, 1.0);
  pad011->Draw();
  pad012->Draw();

  pad011->cd();
  pad011->SetGrid();
  h1_trig->Draw();

  pad012->cd();
  pad012->SetGrid();
  h1_flag_v1495->Draw();

  pad0->cd(2);
  TPad* pad021 = new TPad("pad021", "", 0.0, 0.0, 0.7, 1.0);
  TPad* pad022 = new TPad("pad022", "", 0.7, 0.0, 1.0, 1.0);
  pad021->Draw();
  pad022->Draw();

  pad021->cd();
  pad021->SetGrid();
  h1_evt_qual->Draw();

  pad022->cd();
  pad022->SetGrid();
  h1_n_taiwan->Draw();
  double n_ttdc_mean = h1_n_taiwan->GetMean();
  if (fabs(n_ttdc_mean - n_ttdc) > 0.1) {
    can0->SetWorseStatus(OnlMonCanvas::ERROR);
    can0->AddMessage(TString::Format("N of Taiwan TDCs = %.1f, not %d.", n_ttdc_mean, n_ttdc).Data());
  }

  pad0->cd(3);
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
    can0->SetWorseStatus(OnlMonCanvas::ERROR);
    can0->AddMessage("Errors in >5% of Coda flush events.");
  } else if (n_bad / n_all > 0.01) {
    can0->SetWorseStatus(OnlMonCanvas::WARN);
    can0->AddMessage("Errors in >1% of Coda flush events.");
  }

  n_all = h1_cnt->GetBinContent(8);
  n_bad = h1_cnt->GetBinContent(10);
  oss.str("");
  oss << "N of Taiwan TDC hits = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());
  if (n_bad / n_all > 0.05) {
    can0->SetWorseStatus(OnlMonCanvas::ERROR);
    can0->AddMessage("Errors in >5% of Taiwan TDC hits.");
  } else if (n_bad / n_all > 0.01) {
    can0->SetWorseStatus(OnlMonCanvas::WARN);
    can0->AddMessage("Errors in >1% of Taiwan TDC hits.");
  }

  n_all = h1_cnt->GetBinContent(9);
  n_bad = h1_cnt->GetBinContent(11);
  oss.str("");
  oss << "N of v1495 TDC hits = " << n_all << " (all), " << n_bad << " (bad)";
  pate->AddText(oss.str().c_str());
  if (n_bad / n_all > 0.05) {
    can0->SetWorseStatus(OnlMonCanvas::ERROR);
    can0->AddMessage("Errors in >5% of v1495 TDC hits.");
  } else if (n_bad / n_all > 0.01) {
    can0->SetWorseStatus(OnlMonCanvas::WARN);
    can0->AddMessage("Errors in >1% of v1495 TDC hits.");
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
    can0->SetWorseStatus(OnlMonCanvas::ERROR);
    can0->AddMessage("Errors in >5% of v1495 events.");
  } else if ((n_d1ad + n_d2ad + n_d3ad) / n_all > 0.01) {
    can0->SetWorseStatus(OnlMonCanvas::WARN);
    can0->AddMessage("Errors in >1% of v1495 events.");
  }

  OnlMonCanvas* can1 = GetCanvas(1); // ========== Canvas #1 ==========
  can1->SetStatus(OnlMonCanvas::OK);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetMargin(0.10, 0.10, 0.08, 0.01); // (l, r, b, t)
  pad1->SetGrid();
  pad1->SetLogx();
  pad1->SetLogz();

  const int N_PL = nChamberPlanes + nHodoPlanes + nPropPlanes + nDarkPhotonPlanes;
  int nbin_nhit = h2_nhit_pl->GetNbinsX();
  for (int i_pl = 1; i_pl <= N_PL + 1; i_pl++) {
    double n_uf = h2_nhit_pl->GetBinContent(          0, i_pl);
    double n_of = h2_nhit_pl->GetBinContent(nbin_nhit+1, i_pl);
    h2_nhit_pl->AddBinContent(h2_nhit_pl->GetBin(        1, i_pl), n_uf);
    h2_nhit_pl->AddBinContent(h2_nhit_pl->GetBin(nbin_nhit, i_pl), n_of);
  }
  h2_nhit_pl->GetXaxis()->SetLabelSize(0.03); // default = 0.04
  h2_nhit_pl->GetXaxis()->SetTitleSize(0.03); // default = 0.04
  h2_nhit_pl->GetYaxis()->SetLabelSize(0.03); // default = 0.04
  h2_nhit_pl->GetYaxis()->SetTitleSize(0.03); // default = 0.04
  h2_nhit_pl->GetZaxis()->SetLabelSize(0.03); // default = 0.04
  h2_nhit_pl->GetXaxis()->SetTitleOffset(1.1); // default = 1.0
  h2_nhit_pl->GetZaxis()->SetLabelOffset(0); // default = 0.005
  h2_nhit_pl->Draw("colz");

  h2_nhit_pl->GetYaxis()->SetRange(N_PL+1, N_PL+1);
  double nhit_pl = h2_nhit_pl->GetMean();
  h2_nhit_pl->GetYaxis()->SetRange(); // Reset
  if (nhit_pl > nhit_pl_err) {
    can1->SetWorseStatus(OnlMonCanvas::ERROR);
    can1->AddMessage(TString::Format("N of all-plane hits = %.1f > %d.", nhit_pl, nhit_pl_err).Data());
  } else if (nhit_pl > nhit_pl_warn) {
    can1->SetWorseStatus(OnlMonCanvas::WARN);
    can1->AddMessage(TString::Format("N of all-plane hits = %.1f > %d.", nhit_pl, nhit_pl_warn).Data());
  }
  
  OnlMonCanvas* can2 = GetCanvas(2); // ========== Canvas #2 ==========
  can2->SetStatus(OnlMonCanvas::OK);
  can2->GetMainPad(); 
  TPad* pad21 = new TPad("pad21", "", 0.0, 0.7, 1.0, 1.0);
  TPad* pad22 = new TPad("pad22", "", 0.0, 0.0, 1.0, 0.7);
  pad21->Draw();
  pad22->Draw();

  //TVirtualPad* pad21 = pad2->cd(1);
  pad21->cd();
  pad21->SetMargin(0.08, 0.01, 0.15, 0.01); // (l, r, b, t)
  pad21->SetGrid();
  pad21->SetLogy();

  int nbin_sp = h1_nevt_sp->GetNbinsX();
  unsigned int sp_id0 = (unsigned int)(h1_nevt_sp->GetBinContent(nbin_sp + 1) / h1_nevt_sp->GetBinContent(0));
  oss.str("");
  oss << "Spill ID #minus_{} " << sp_id0;
  string title_sp_id = oss.str();

  h1_nevt_sp->GetXaxis()->SetTitle(title_sp_id.c_str());
  UtilHist::AutoSetRange(h1_nevt_sp, 0, 0);
  h1_nevt_sp->GetXaxis()->SetLabelSize(0.08); // default = 0.04
  h1_nevt_sp->GetXaxis()->SetTitleSize(0.08); // default = 0.04
  h1_nevt_sp->GetYaxis()->SetLabelSize(0.08); // default = 0.04
  h1_nevt_sp->GetYaxis()->SetTitleSize(0.08); // default = 0.04
  h1_nevt_sp->GetYaxis()->SetTitleOffset(0.53); // default = 1.0
  h1_nevt_sp->SetMarkerStyle(7);
  h1_nevt_sp->Draw("pl");

  double nevt_sp = EvalAverageOfFilledBins(h1_nevt_sp);
  if (nevt_sp > nevt_sp_err) {
    can2->SetWorseStatus(OnlMonCanvas::ERROR);
    can2->AddMessage(TString::Format("N of events/spill = %.1f > %d.", nevt_sp, nevt_sp_err).Data());
  } else if (nevt_sp > nevt_sp_warn) {
    can2->SetWorseStatus(OnlMonCanvas::WARN);
    can2->AddMessage(TString::Format("N of events/spill = %.1f > %d.", nevt_sp, nevt_sp_warn).Data());
  }

  pad22->cd();
  pad22->SetMargin(0.08, 0.01, 0.10, 0.10); // (l, r, b, t)
  pad22->SetGrid();
  pad22->SetLogy();

  TH1* h1[5] = { h1_time_input, h1_time_deco, h1_time_map, h1_time_subsys, h1_time_output };
  const char* label_h1[5] = {
    "Wait/read input",
    "Decode words"   ,
    "Map channels"   ,
    "Process events" ,
    "Write outputs"   };

  int bin_lo, bin_hi;
  UtilHist::FindFilledRange(h1_time_input, bin_lo, bin_hi);

  THStack* hs23 = new THStack("hs23", (";" + title_sp_id + ";Time (sec)").c_str());
  for (int ih = 0; ih < 5; ih++) {
    int color = ih < 4  ?  ih+1  :  ih+2; // Skip yellow (5).
    h1[ih]->SetLineColor  (color);
    h1[ih]->SetMarkerColor(color);
    h1[ih]->SetMarkerStyle(7);
    hs23->Add(h1[ih], "pl");
  }
  hs23->Draw("nostack");
  hs23->GetXaxis()->SetLabelSize(0.035); // default = 0.04
  hs23->GetXaxis()->SetTitleSize(0.035); // default = 0.04
  hs23->GetYaxis()->SetLabelSize(0.035); // default = 0.04
  hs23->GetYaxis()->SetTitleSize(0.035); // default = 0.04
  hs23->GetYaxis()->SetTitleOffset(1.1); // default = 1.0
  hs23->GetXaxis()->SetRange(bin_lo, bin_hi);
  for (int ih = 0; ih < 5; ih++) {
    double x0 = 0.09 + 0.30 * (ih % 3);
    double y0 = 0.94 - 0.04 * (ih / 3);
    TLegend* leg = new TLegend(x0, y0, x0+0.3, y0+0.05);
    leg->AddEntry(h1[ih], label_h1[ih], "pl");
    leg->SetTextSize(0.035);
    leg->SetLineWidth(0);
    leg->SetFillStyle(0);
    leg->Draw();
  }
  for (int ih = 0; ih < 5; ih++) {
    double time = EvalAverageOfFilledBins(h1[ih]);
    if (time > 20) {
      can2->SetWorseStatus(OnlMonCanvas::ERROR);
      can2->AddMessage(TString::Format("Time to %s = %.1f > 20.", label_h1[ih], time).Data());
    } else if (time > 10) {
      can2->SetWorseStatus(OnlMonCanvas::WARN);
      can2->AddMessage(TString::Format("Time to %s = %.1f > 10.", label_h1[ih], time).Data());
    }
  }

  return 0;
}

double OnlMonMainDaq::EvalAverageOfFilledBins(TH1* h1) const
{
  double cont = 0;
  int    num  = 0;
  for (int ii = 1; ii <= h1->GetNbinsX(); ii++) {
    double c = h1->GetBinContent(ii);
    if (c == 0) continue;
    cont += c;
    num++;
  }
  return num > 0 ? cont/num : 0;
}
