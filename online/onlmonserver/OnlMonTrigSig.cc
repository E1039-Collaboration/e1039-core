/// OnlMonTrigSig.C
#include <iomanip>
#include <TH2D.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include "OnlMonServer.h"
#include "OnlMonTrigSig.h"
#include "UtilHist.h"
using namespace std;

// BeforeInhNIM
// BeforeInhMatrix
// AfterInhNIM
// AfterInhMatrix 
// BOS  (ch = ?)
// EOS  (ch = ?)
// L1   (ch = ?)
// RF   (ch = 0-8 on v1495)
// STOP (ch = 0-8 on v1495)
// L1PXtp
// L1PXtn
// L1PXbp
// L1PXbn
// L1NIMxt
// L1NIMxb
// L1NIMyt
// L1NIMyb

OnlMonTrigSig::OnlMonTrigSig()
{
  NumCanvases(3);
  Name("OnlMonTrigSig");
  Title("Trigger Signal");
}

int OnlMonTrigSig::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigSig::InitRunOnlMon(PHCompositeNode* topNode)
{
  const double DT = 4/9.0; // 4/9 ns per single count of Taiwan TDC
  const int NT = 4000;
  const double T0 = 0.5*DT;
  const double T1 = (NT+0.5)*DT;
  h2_bi_fpga = new TH2D("h2_bi_fpga", "FPGA Before Inhibit;tdcTime;", NT, T0, T1,  5, 0.5, 5.5);
  h2_ai_fpga = new TH2D("h2_ai_fpga",  "FPGA After Inhibit;tdcTime;", NT, T0, T1,  5, 0.5, 5.5);
  h2_bi_nim  = new TH2D("h2_bi_nim" ,  "NIM Before Inhibit;tdcTime;", NT, T0, T1,  5, 0.5, 5.5);
  h2_ai_nim  = new TH2D("h2_ai_nim" ,   "NIM After Inhibit;tdcTime;", NT, T0, T1,  5, 0.5, 5.5);
  for (int ii = 1; ii <= 5; ii++) {
    ostringstream oss;
    oss << "FPGA " << ii;
    h2_bi_fpga->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
    h2_ai_fpga->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
    oss.str("");
    oss << "NIM " << ii;
    h2_bi_nim->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
    h2_ai_nim->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
  }

  const double DT2 = 1.0; // 1 ns per single count of v1495 TDC
  const int NT2 = 2000;
  const double T02 = 0.5*DT2;
  const double T12 = (NT2+0.5)*DT2;
  h2_rf      = new TH2D("h2_rf"     ,   "RF on v1495;tdcTime;", NT2, T02, T12,  9, 0.5, 9.5);
  h2_stop    = new TH2D("h2_stop"   , "STOP on v1495;tdcTime;", NT2, T02, T12,  9, 0.5, 9.5);
  for (int ii = 1; ii <= 9; ii++) {
    ostringstream oss;
    oss << "Board " << ii;
    h2_rf  ->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
    h2_stop->GetYaxis()->SetBinLabel(ii, oss.str().c_str());
  }

  RegisterHist(h2_bi_fpga);
  RegisterHist(h2_ai_fpga);
  RegisterHist(h2_bi_nim );
  RegisterHist(h2_ai_nim );
  RegisterHist(h2_rf     );
  RegisterHist(h2_stop   );

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigSig::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!event_header || !hit_vec || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  GeomSvc* geom = GeomSvc::instance();

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int det_id  = (*it)->get_detector_id();
    int ele_id  = (*it)->get_element_id ();
    double time = (*it)->get_tdc_time   ();
    if      (det_id == geom->getDetectorID("BeforeInhNIM"   )) h2_bi_nim ->Fill(time, ele_id);
    else if (det_id == geom->getDetectorID("BeforeInhMatrix")) h2_bi_fpga->Fill(time, ele_id);
    else if (det_id == geom->getDetectorID("AfterInhNIM"    )) h2_ai_nim ->Fill(time, ele_id);
    else if (det_id == geom->getDetectorID("AfterInhMatrix" )) h2_ai_fpga->Fill(time, ele_id);
  }

  for (SQHitVector::ConstIter it = trig_hit_vec->begin(); it != trig_hit_vec->end(); it++) {
    int det_id  = (*it)->get_detector_id();
    int ele_id  = (*it)->get_element_id ();
    double time = (*it)->get_tdc_time   ();
    if      (det_id == geom->getDetectorID("RF"  )) h2_rf  ->Fill(time, ele_id);
    else if (det_id == geom->getDetectorID("STOP")) h2_stop->Fill(time, ele_id);
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigSig::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigSig::FindAllMonHist()
{
  h2_bi_fpga = (TH2*)FindMonObj("h2_bi_fpga");
  h2_ai_fpga = (TH2*)FindMonObj("h2_ai_fpga");
  h2_bi_nim  = (TH2*)FindMonObj("h2_bi_nim" );
  h2_ai_nim  = (TH2*)FindMonObj("h2_ai_nim" );
  h2_rf      = (TH2*)FindMonObj("h2_rf"     );
  h2_stop    = (TH2*)FindMonObj("h2_stop"   );
  return (h2_bi_fpga && h2_ai_fpga && h2_bi_nim && h2_ai_nim && h2_rf && h2_stop  ?  0  :  1);
}

int OnlMonTrigSig::DrawMonitor()
{
  UtilHist::AutoSetRangeX(h2_bi_fpga);
  UtilHist::AutoSetRangeX(h2_ai_fpga);
  UtilHist::AutoSetRangeX(h2_bi_nim );
  UtilHist::AutoSetRangeX(h2_ai_nim );
  UtilHist::AutoSetRangeX(h2_rf     );
  UtilHist::AutoSetRangeX(h2_stop   );

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(1, 2);
  pad0->cd(1);  DrawTH2WithPeakPos(h2_bi_fpga);
  pad0->cd(2);  DrawTH2WithPeakPos(h2_ai_fpga);
  can0->AddMessage("OK");
  can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(1, 2);
  pad1->cd(1);  DrawTH2WithPeakPos(h2_bi_nim);
  pad1->cd(2);  DrawTH2WithPeakPos(h2_ai_nim);
  can1->AddMessage("OK");
  can1->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can2 = GetCanvas(2);
  TPad* pad2 = can2->GetMainPad();
  pad2->SetGrid();
  pad2->Divide(1, 2);
  pad2->cd(1);  DrawTH2WithPeakPos(h2_rf  );
  pad2->cd(2);  DrawTH2WithPeakPos(h2_stop);
  can2->AddMessage("OK");
  can2->SetStatus(OnlMonCanvas::OK);

  return 0;
}

void OnlMonTrigSig::DrawTH2WithPeakPos(TH2* h2, const double cont_min)
{
  h2->Draw("colz");
  int ny = h2->GetNbinsY();
  for (int iy = 1; iy <= ny; iy++) {
    TH1* h1 = h2->ProjectionX("h1_draw_th2", iy, iy);
    ostringstream oss;
    if (h1->GetMaximum() >= cont_min) {
      oss << "Peak @ " << h1->GetXaxis()->GetBinCenter(h1->GetMaximumBin());
    } else {
      oss << "No sizable peak";
    }
    TText* text = new TText();
    text->SetNDC(true);
    text->SetTextAlign(22);
    text->DrawText(0.3, 0.1+(iy-0.5)*0.8/ny, oss.str().c_str());
    // The y-position above assumes that the top & bottom margins are 0.1 each.
  }
}
