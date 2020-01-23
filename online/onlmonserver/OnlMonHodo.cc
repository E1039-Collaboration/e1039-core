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
//#include <geom_svc/CalibParamInTimeTaiwan.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonServer.h"
#include "OnlMonHodo.h"
using namespace std;

OnlMonHodo::OnlMonHodo(const HodoType_t type) : m_type(type)
{
  NumCanvases(2);
  m_n_pl = 2;
  switch (m_type) {
  case H1X:  m_pl0 = 31;  Name("OnlMonHodoH1X" );  Title("Hodo: H1X" );  break;
  case H2X:  m_pl0 = 37;  Name("OnlMonHodoH2X" );  Title("Hodo: H2X" );  break;
  case H3X:  m_pl0 = 39;  Name("OnlMonHodoH3X" );  Title("Hodo: H3X" );  break;
  case H4X:  m_pl0 = 45;  Name("OnlMonHodoH4X" );  Title("Hodo: H4X" );  break;
  case H1Y:  m_pl0 = 33;  Name("OnlMonHodoH1Y" );  Title("Hodo: H1Y" );  break;
  case H2Y:  m_pl0 = 35;  Name("OnlMonHodoH2Y" );  Title("Hodo: H2Y" );  break;
  case H4Y1: m_pl0 = 41;  Name("OnlMonHodoH4Y1");  Title("Hodo: H4Y1");  break;
  case H4Y2: m_pl0 = 43;  Name("OnlMonHodoH4Y2");  Title("Hodo: H4Y2");  break;
  case DP1T: m_pl0 = 55;  Name("OnlMonHodoDP1T");  Title("Hodo: DP1T");  break;
  case DP1B: m_pl0 = 57;  Name("OnlMonHodoDP1B");  Title("Hodo: DP1B");  break;
  case DP2T: m_pl0 = 59;  Name("OnlMonHodoDP2T");  Title("Hodo: DP2T");  break;
  case DP2B: m_pl0 = 61;  Name("OnlMonHodoDP2B");  Title("Hodo: DP2B");  break;
  }
}

int OnlMonHodo::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::InitRunOnlMon(PHCompositeNode* topNode)
{
  //SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  //if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;

  GeomSvc* geom = GeomSvc::instance();
  //CalibParamInTimeTaiwan calib;
  //calib.SetMapIDbyDB(run_header->get_run_id());
  //calib.ReadFromDB();

  ostringstream oss;
  for (int pl = 0; pl < m_n_pl; pl++) {
    string name = geom->getDetectorName(m_pl0 + pl);
    int n_ele = geom->getPlaneNElements(m_pl0 + pl); 
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;Hit count";
    h1_ele[pl]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_ele_in_" << pl;
    h1_ele_in[pl] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;In-time hit count";
    h1_ele_in[pl]->SetTitle(oss.str().c_str());

    const double DT = 20/9.0; // 4/9 ns per single count of Taiwan TDC
    const int    NT = 200;
    const double T0 = 200.5*DT;
    const double T1 = 400.5*DT;

    //double center, width;
    //calib.Find(m_pl0 + pl, 1, center, width);
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[pl]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_in_" << pl;
    h1_time_in[pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;In-time hit count";
    h1_time_in[pl]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h2_time_ele_" << pl;
    h2_time_ele[pl] = new TH2D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5, NT, T0, T1);
    oss.str("");
    oss << name << ";Element ID;tdcTime;Hit count";
    h2_time_ele[pl]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele    [pl]);
    RegisterHist(h1_ele_in [pl]);
    RegisterHist(h1_time   [pl]);
    RegisterHist(h1_time_in[pl]);
    RegisterHist(h2_time_ele[pl]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonHodo::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int pl = (*it)->get_detector_id() - m_pl0;
    if (pl < 0 || pl >= m_n_pl) continue;
    int    ele  = (*it)->get_element_id();
    double time = (*it)->get_tdc_time  ();
    h1_ele [pl]->Fill(ele );
    h1_time[pl]->Fill(time);
    h2_time_ele[pl]->Fill(ele, time);
    if ((*it)->is_in_time()) {
      h1_ele_in [pl]->Fill(ele );
      h1_time_in[pl]->Fill(time);
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
  for (int pl = 0; pl < m_n_pl; pl++) {
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = FindMonHist(oss.str().c_str());
    if (! h1_ele[pl]) return 1;
    oss.str("");
    oss << "h1_ele_in_" << pl;
    h1_ele_in[pl] = FindMonHist(oss.str().c_str());
    if (! h1_ele_in[pl]) return 1;
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = FindMonHist(oss.str().c_str());
    if (! h1_time[pl]) return 1;
    oss.str("");
    oss << "h1_time_in_" << pl;
    h1_time_in[pl] = FindMonHist(oss.str().c_str());
    if (! h1_time_in[pl]) return 1;
    oss.str("");
    oss << "h2_time_ele_" << pl;
    h2_time_ele[pl] = (TH2*)FindMonHist(oss.str().c_str());
    if (! h2_time_ele[pl]) return 1;
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
  for (int pl = 0; pl < m_n_pl; pl++) {
    pad0->cd(2*pl+1);
    h1_ele[pl]->SetLineColor(kBlack);
    h1_ele[pl]->Draw();
    h1_ele_in[pl]->SetLineColor(kBlue);
    h1_ele_in[pl]->SetFillColor(kBlue-7);
    h1_ele_in[pl]->Draw("same");
    if (h1_ele[pl]->GetMinimum() == 0) empty_ele = true;

    pad0->cd(2*pl+2);
    h2_time_ele[pl]->Draw("colz");
    ostringstream oss;
    oss << "pr_" << h2_time_ele[pl]->GetName();
    TProfile* pr = h2_time_ele[pl]->ProfileX(oss.str().c_str(), 1, -1, "s");
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
  for (int pl = 0; pl < m_n_pl; pl++) {
    pad1->cd(pl+1);
    //UtilHist::AutoSetRange(h1_time[pl]);
    //h1_time[pl]->GetXaxis()->SetRangeUser(400, 1050);
    h1_time[pl]->SetLineColor(kBlack);
    h1_time[pl]->Draw();
    h1_time_in[pl]->SetLineColor(kBlue);
    h1_time_in[pl]->SetFillColor(kBlue-7);
    h1_time_in[pl]->Draw("same");
  }

  return 0;
}
