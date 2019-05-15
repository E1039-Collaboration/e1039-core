/// OnlMonTrigSig.C
#include <iomanip>
#include <TH2D.h>
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
#include <geom_svc/GeomSvc.h>
#include <chan_map/CalibParamInTimeTaiwan.h>
#include "OnlMonServer.h"
#include "OnlMonTrigSig.h"
#include "UtilHist.h"
using namespace std;

// BeforeInhNIM = 55
// BeforeInhMatrix
// AfterInhNIM
// AfterInhMatrix 
// BOS ???
// EOS ???
// L1  ???
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
  NumCanvases(2);
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

  Fun4AllHistoManager* hm = new Fun4AllHistoManager(Name());
  OnlMonServer::instance()->registerHistoManager(hm);
  hm->registerHisto(h2_bi_fpga);
  hm->registerHisto(h2_ai_fpga);
  hm->registerHisto(h2_bi_nim );
  hm->registerHisto(h2_ai_nim );

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonTrigSig::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int det_id  = (*it)->get_detector_id();
    int ele_id  = (*it)->get_element_id ();
    double time = (*it)->get_tdc_time   ();
    /// Lazy ID search.  Temporary...
    if      (det_id == 55) h2_bi_nim ->Fill(time, ele_id);
    else if (det_id == 56) h2_bi_fpga->Fill(time, ele_id);
    else if (det_id == 57) h2_ai_nim ->Fill(time, ele_id);
    else if (det_id == 58) h2_ai_fpga->Fill(time, ele_id);
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
  return (h2_bi_fpga && h2_ai_fpga && h2_bi_nim && h2_ai_nim  ?  0  :  1);
}

int OnlMonTrigSig::DrawMonitor()
{
  UtilHist::AutoSetRangeX(h2_bi_fpga);
  UtilHist::AutoSetRangeX(h2_ai_fpga);
  UtilHist::AutoSetRangeX(h2_bi_nim );
  UtilHist::AutoSetRangeX(h2_ai_nim );

  //TH1* h1_tmp = h2_bi_fpga->ProjectionX("h1_tmp");
  //int i_lo = 1;
  //while (h1_tmp->GetBinContent(i_lo) == 0) i_lo++;
  //int i_hi = h1_tmp->GetNbinsX();
  //while (h1_tmp->GetBinContent(i_hi) == 0) i_hi--;
  //delete h1_tmp;
  //cout << "L " << i_lo << " " << i_hi << endl;
  //h2_bi_fpga->GetXaxis()->SetRange(i_lo-5, i_hi+5);

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(1, 2);
  pad0->cd(1);  h2_bi_fpga->Draw("colz");
  pad0->cd(2);  h2_ai_fpga->Draw("colz");
  can0->AddMessage("Always Okay ;^D");
  can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(1, 2);
  pad1->cd(1);  h2_bi_nim ->Draw("colz");
  pad1->cd(2);  h2_ai_nim ->Draw("colz");
  can1->AddMessage("Always Okay ;^D");
  can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}
