/// OnlMonHodo.C
#include <iomanip>
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonServer.h"
#include "OnlMonHodo.h"
using namespace std;

OnlMonHodo::OnlMonHodo(const HodoType_t type) : m_type(type)
{
  NumCanvases(2);
  switch (m_type) {
  case H1X:  Name("OnlMonHodoH1X" );  Title("Hodo: H1X" );  break;
  case H2X:  Name("OnlMonHodoH2X" );  Title("Hodo: H2X" );  break;
  case H3X:  Name("OnlMonHodoH3X" );  Title("Hodo: H3X" );  break;
  case H4X:  Name("OnlMonHodoH4X" );  Title("Hodo: H4X" );  break;
  case H1Y:  Name("OnlMonHodoH1Y" );  Title("Hodo: H1Y" );  break;
  case H2Y:  Name("OnlMonHodoH2Y" );  Title("Hodo: H2Y" );  break;
  case H4Y1: Name("OnlMonHodoH4Y1");  Title("Hodo: H4Y1");  break;
  case H4Y2: Name("OnlMonHodoH4Y2");  Title("Hodo: H4Y2");  break;
  case DP1T: Name("OnlMonHodoDP1T");  Title("Hodo: DP1T");  break;
  case DP1B: Name("OnlMonHodoDP1B");  Title("Hodo: DP1B");  break;
  case DP2T: Name("OnlMonHodoDP2T");  Title("Hodo: DP2T");  break;
  case DP2B: Name("OnlMonHodoDP2B");  Title("Hodo: DP2B");  break;
  }
}

int OnlMonHodo::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::InitRunOnlMon(PHCompositeNode* topNode)
{
  const double DT = 20/9.0; // 4/9 ns per single count of Taiwan TDC
  int NT    = 700;
  double T0 = 200.5*DT;
  double T1 = 900.5*DT;
  switch (m_type) {
  case H1X:  SetDet("H1T"  ,"H1B"  ); break;
  case H2X:  SetDet("H2T"  ,"H2B"  ); break;
  case H3X:  SetDet("H3T"  ,"H3B"  ); break;
  case H4X:  SetDet("H4T"  ,"H4B"  ); break;
  case H1Y:  SetDet("H1L"  ,"H1R"  ); break;
  case H2Y:  SetDet("H2L"  ,"H2R"  ); break;
  case H4Y1: SetDet("H4Y1L","H4Y1R"); break;
  case H4Y2: SetDet("H4Y2L","H4Y2R"); break;
  case DP1T: SetDet("DP1TL","DP1TR"); break;
  case DP1B: SetDet("DP1BL","DP1BR"); break;
  case DP2T: SetDet("DP2TL","DP2TR"); break;
  case DP2B: SetDet("DP2BL","DP2BR"); break;
//  case DP1T: SetDet("DP1TL","DP1TR"); NT=200; T0=300.5*DT; T1=500.5*DT; break;
//  case DP1B: SetDet("DP1BL","DP1BR"); NT=200; T0=300.5*DT; T1=500.5*DT; break;
//  case DP2T: SetDet("DP2TL","DP2TR"); NT=200; T0=300.5*DT; T1=500.5*DT; break;
//  case DP2B: SetDet("DP2BL","DP2BR"); NT=200; T0=300.5*DT; T1=500.5*DT; break;
  }

  GeomSvc* geom = GeomSvc::instance();
  ostringstream oss;
  for (unsigned int i_det = 0; i_det < N_DET; i_det++) {
    string name = list_det_name[i_det];
    int  det_id = list_det_id  [i_det];
    int n_ele  = geom->getPlaneNElements(det_id);
    if (det_id <= 0 || n_ele <= 0) {
      cout << "OnlMonHodo::InitRunOnlMon():  Invalid det_id or n_ele: " 
           << det_id << " " << n_ele << " at name = " << name << "." << endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

    oss.str("");
    oss << "h1_ele_" << i_det;
    h1_ele[i_det] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;Hit count";
    h1_ele[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_ele_in_" << i_det;
    h1_ele_in[i_det] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;In-time hit count";
    h1_ele_in[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_" << i_det;
    h1_time[i_det] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_in_" << i_det;
    h1_time_in[i_det] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;In-time hit count";
    h1_time_in[i_det]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h2_time_ele_" << i_det;
    h2_time_ele[i_det] = new TH2D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5, NT, T0, T1);
    oss.str("");
    oss << name << ";Element ID;tdcTime;Hit count";
    h2_time_ele[i_det]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele     [i_det]);
    RegisterHist(h1_ele_in  [i_det]);
    RegisterHist(h1_time    [i_det]);
    RegisterHist(h1_time_in [i_det]);
    RegisterHist(h2_time_ele[i_det]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int det_id = (*it)->get_detector_id();
    for (int i_det = 0; i_det < N_DET; i_det++) {
      if (list_det_id[i_det] != det_id) continue;
      int    ele  = (*it)->get_element_id();
      double time = (*it)->get_tdc_time  ();
      h1_ele     [i_det]->Fill(ele );
      h1_time    [i_det]->Fill(time);
      h2_time_ele[i_det]->Fill(ele, time);
      if ((*it)->is_in_time()) {
        h1_ele_in [i_det]->Fill(ele );
        h1_time_in[i_det]->Fill(time);
      }
    }
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::FindAllMonHist()
{
  ostringstream oss;
  for (int i_det = 0; i_det < N_DET; i_det++) {
    oss.str("");
    oss << "h1_ele_" << i_det;
    h1_ele[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_ele[i_det]) return 1;
    oss.str("");
    oss << "h1_ele_in_" << i_det;
    h1_ele_in[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_ele_in[i_det]) return 1;
    oss.str("");
    oss << "h1_time_" << i_det;
    h1_time[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_time[i_det]) return 1;
    oss.str("");
    oss << "h1_time_in_" << i_det;
    h1_time_in[i_det] = FindMonHist(oss.str().c_str());
    if (! h1_time_in[i_det]) return 1;
    oss.str("");
    oss << "h2_time_ele_" << i_det;
    h2_time_ele[i_det] = (TH2*)FindMonHist(oss.str().c_str());
    if (! h2_time_ele[i_det]) return 1;
  }
  return 0;
}

int OnlMonHodo::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  can0->SetStatus(OnlMonCanvas::OK);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(2, 2);
  bool empty_ele = false;
  for (int i_det = 0; i_det < N_DET; i_det++) {
    pad0->cd(2*i_det+1);
    h1_ele[i_det]->SetLineColor(kBlack);
    h1_ele[i_det]->Draw();
    h1_ele_in[i_det]->SetLineColor(kBlue);
    h1_ele_in[i_det]->SetFillColor(kBlue-7);
    h1_ele_in[i_det]->Draw("same");
    if (h1_ele[i_det]->GetMinimum() == 0) empty_ele = true;

    pad0->cd(2*i_det+2);
    UtilHist::AutoSetRangeY(h2_time_ele[i_det]);
    h2_time_ele[i_det]->Draw("colz");
    ostringstream oss;
    oss << "pr_" << h2_time_ele[i_det]->GetName();
    TProfile* pr = h2_time_ele[i_det]->ProfileX(oss.str().c_str()); // , 1, -1, "s");
    pr->SetLineColor(kBlack);
    pr->Draw("E1same");
  }
  if (empty_ele) {
    can0->SetStatus(OnlMonCanvas::WARN);
    can0->AddMessage("No-hit element.");
  }

  OnlMonCanvas* can1 = GetCanvas(1);
  //can1->SetStatus(OnlMonCanvas::OK);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(1, 2);
  for (int i_det = 0; i_det < N_DET; i_det++) {
    pad1->cd(i_det+1);
    UtilHist::AutoSetRange(h1_time[i_det]);
    //h1_time[i_det]->GetXaxis()->SetRangeUser(400, 1050);
    h1_time[i_det]->SetLineColor(kBlack);
    h1_time[i_det]->Draw();
    h1_time_in[i_det]->SetLineColor(kBlue);
    h1_time_in[i_det]->SetFillColor(kBlue-7);
    h1_time_in[i_det]->Draw("same");
  }

  return 0;
}

void OnlMonHodo::SetDet(const char* det0, const char* det1)
{
  list_det_name[0] = det0;
  list_det_name[1] = det1;
  GeomSvc* geom = GeomSvc::instance();
  for (int ii = 0; ii < N_DET; ii++) {
    list_det_id[ii] = geom->getDetectorID(list_det_name[ii]);
  }
}
