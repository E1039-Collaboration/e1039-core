/// OnlMonV1495.C
#include <iomanip>
#include <TH1D.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
//#include <geom_svc/CalibParamInTimeV1495.h>
#include "OnlMonServer.h"
#include "OnlMonV1495.h"
#include "UtilHist.h"
using namespace std;

OnlMonV1495::OnlMonV1495(const HodoType_t type, const int lvl) : m_type(type), m_lvl(lvl)
{
  NumCanvases(2);
  m_n_pl = 2;
  switch (m_type) {
  case H1X:  m_pl0 = 31;  Name("OnlMonV1495H1X" );  Title("V1495: H1X" );  break;
  case H2X:  m_pl0 = 37;  Name("OnlMonV1495H2X" );  Title("V1495: H2X" );  break;
  case H3X:  m_pl0 = 39;  Name("OnlMonV1495H3X" );  Title("V1495: H3X" );  break;
  case H4X:  m_pl0 = 45;  Name("OnlMonV1495H4X" );  Title("V1495: H4X" );  break;
  case H1Y:  m_pl0 = 33;  Name("OnlMonV1495H1Y" );  Title("V1495: H1Y" );  break;
  case H2Y:  m_pl0 = 35;  Name("OnlMonV1495H2Y" );  Title("V1495: H2Y" );  break;
  case H4Y1: m_pl0 = 41;  Name("OnlMonV1495H4Y1");  Title("V1495: H4Y1");  break;
  case H4Y2: m_pl0 = 43;  Name("OnlMonV1495H4Y2");  Title("V1495: H4Y2");  break;
  }
}

int OnlMonV1495::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::InitRunOnlMon(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;

  GeomSvc* geom = GeomSvc::instance();
  //CalibParamInTimeV1495 calib;
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

    const double DT = 1.0; // 1 ns per single count of v1495 TDC
    const int NT = 2000;
    const double T0 = 0.5*DT;
    const double T1 = (NT+0.5)*DT;

    //double center, width;
    //calib.Find(m_pl0 + pl, 1, m_lvl, center, width);
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    //h1_time[pl] = new TH1D(oss.str().c_str(), "", 100, center-2.5*width, center+2.5*width);

    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[pl]->SetTitle(oss.str().c_str());

    oss.str("");
    oss << "h1_time_in_" << pl;
    h1_time_in[pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << name << ";tdcTime;In-time hit count";
    h1_time_in[pl]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele    [pl]);
    RegisterHist(h1_ele_in [pl]);
    RegisterHist(h1_time   [pl]);
    RegisterHist(h1_time_in[pl]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int pl = (*it)->get_detector_id() - m_pl0;
    if (pl < 0 || pl >= m_n_pl || (*it)->get_level() != m_lvl) continue;
    h1_ele [pl]->Fill((*it)->get_element_id());
    h1_time[pl]->Fill((*it)->get_tdc_time  ());
    if ((*it)->is_in_time()) {
      h1_ele_in [pl]->Fill((*it)->get_element_id());
      h1_time_in[pl]->Fill((*it)->get_tdc_time());
    }
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonV1495::FindAllMonHist()
{
  ostringstream oss;
  for (int pl = 0; pl < m_n_pl; pl++) {
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = (TH1*)FindMonObj(oss.str().c_str());
    if (! h1_ele[pl]) return 1;
    oss.str("");
    oss << "h1_ele_in_" << pl;
    h1_ele_in[pl] = (TH1*)FindMonObj(oss.str().c_str());
    if (! h1_ele_in[pl]) return 1;
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = (TH1*)FindMonObj(oss.str().c_str());
    if (! h1_time[pl]) return 1;
    oss.str("");
    oss << "h1_time_in_" << pl;
    h1_time_in[pl] = (TH1*)FindMonObj(oss.str().c_str());
    if (! h1_time_in[pl]) return 1;
  }
  return 0;
}

int OnlMonV1495::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(1, 2);
  for (int pl = 0; pl < m_n_pl; pl++) {
    pad0->cd(pl+1);
    h1_ele[pl]->SetLineColor(kBlack);
    h1_ele[pl]->Draw();
    h1_ele_in[pl]->SetLineColor(kBlue);
    h1_ele_in[pl]->SetFillColor(kBlue-7);
    h1_ele_in[pl]->Draw("same");
  }
  can0->AddMessage("OK");
  can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(1, 2);
  for (int pl = 0; pl < m_n_pl; pl++) {
    pad1->cd(pl+1);
    UtilHist::AutoSetRange(h1_time[pl]);
    h1_time[pl]->SetLineColor(kBlack);
    h1_time[pl]->Draw();
    h1_time_in[pl]->SetLineColor(kBlue);
    h1_time_in[pl]->SetFillColor(kBlue-7);
    h1_time_in[pl]->Draw("same");
  }
  can1->AddMessage("OK");
  can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}
