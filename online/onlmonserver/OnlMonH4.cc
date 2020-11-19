/// OnlMonH4.C
#include <iomanip>
#include <TH1D.h>
#include <TH2D.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include <UtilAna/UtilHist.h>
#include <UtilAna/UtilSQHit.h>
#include "OnlMonServer.h"
#include "OnlMonH4.h"
using namespace std;

OnlMonH4::OnlMonH4(const HodoType_t type) : m_type(type)
{
  NumCanvases(2);
  switch (m_type) {
  case H4T: 
    Name("OnlMonH4T");
    Title("H4T PMT");
    m_det_name = "H4T";
    m_list_det_name.push_back("H4Tu");
    m_list_det_name.push_back("H4Td");
    break;
  case H4B: 
    Name("OnlMonH4B");
    Title("H4B PMT");
    m_det_name = "H4B";
    m_list_det_name.push_back("H4Bu");
    m_list_det_name.push_back("H4Bd");
    break;
  case H4Y1L:
    Name("OnlMonH4Y1L");
    Title("H4Y1L PMT");
    m_det_name = "H4Y1L";
    m_list_det_name.push_back("H4Y1Ll");
    m_list_det_name.push_back("H4Y1Lr");
    break;
  case H4Y1R:
    Name("OnlMonH4Y1R");
    Title("H4Y1R PMT");
    m_det_name = "H4Y1R";
    m_list_det_name.push_back("H4Y1Rl");
    m_list_det_name.push_back("H4Y1Rr");
    break;
  case H4Y2L:
    Name("OnlMonH4Y2L");
    Title("H4Y2L PMT");
    m_det_name = "H4Y2L";
    m_list_det_name.push_back("H4Y2Ll");
    m_list_det_name.push_back("H4Y2Lr");
    break;
  case H4Y2R:
    Name("OnlMonH4Y2R");
    Title("H4Y2R PMT");
    m_det_name = "H4Y2R";
    m_list_det_name.push_back("H4Y2Rl");
    m_list_det_name.push_back("H4Y2Rr");
    break;
  }
  // m_list_det_name.size() must be N_PL (=2)
}

int OnlMonH4::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonH4::InitRunOnlMon(PHCompositeNode* topNode)
{
  const double DT = 40/9.0; // 4/9 ns per single count of Taiwan TDC
  const int NT    = 350;
  const double T0 = 100.5*DT;
  const double T1 = 450.5*DT;

  GeomSvc* geom = GeomSvc::instance();
  int n_ele = geom->getPlaneNElements( geom->getDetectorID(m_det_name) );

  string det_name[N_PL];
  ostringstream oss;
  m_list_det.clear();
  for (int i_pl = 0; i_pl < N_PL; i_pl++) {
    det_name[i_pl] = m_list_det_name[i_pl];
    short  det_id = geom->getDetectorID(det_name[i_pl]);
    m_list_det.push_back(det_id);

    oss.str("");
    oss << "h1_ele_" << i_pl;
    h1_ele[i_pl] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << det_name[i_pl] << ";Element ID;Hit count";
    h1_ele[i_pl]->SetTitle(oss.str().c_str());


    oss.str("");
    oss << "h1_time_" << i_pl;
    h1_time[i_pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);
    oss.str("");
    oss << det_name[i_pl] << ";tdcTime;Hit count";
    h1_time[i_pl]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele [i_pl]);
    RegisterHist(h1_time[i_pl]);
  }

  h2_ele = new TH2D("h2_ele", "", n_ele, 0.5, n_ele+0.5,  n_ele, 0.5, n_ele+0.5);
  oss.str("");
  oss << ";Element ID of " << det_name[0] << ";Element ID of " << det_name[1] << ";Hit count";
  h2_ele->SetTitle(oss.str().c_str());

  h2_time = new TH2D("h2_time", "", NT, T0, T1,  NT, T0, T1);
  oss.str("");
  oss << ";tdcTime of " << det_name[0] << ";tdcTime ID of " << det_name[1] << ";Hit count";
  h2_time->SetTitle(oss.str().c_str());

  RegisterHist(h2_ele );
  RegisterHist(h2_time);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonH4::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  SQHitVector* hv[N_PL];
  for (int i_pl = 0; i_pl < N_PL; i_pl++) {
    hv[i_pl] = UtilSQHit::FindHits(hit_vec, m_list_det[i_pl]);
    for (SQHitVector::ConstIter it = hv[i_pl]->begin(); it != hv[i_pl]->end(); it++) {
      h1_ele [i_pl]->Fill((*it)->get_element_id());
      h1_time[i_pl]->Fill((*it)->get_tdc_time  ());
    }
  }
  if (hv[0]->size() == 1 && hv[1]->size() == 1) {
    h2_ele ->Fill( hv[0]->at(0)->get_element_id(), hv[1]->at(0)->get_element_id() );
    h2_time->Fill( hv[0]->at(0)->get_tdc_time  (), hv[1]->at(0)->get_tdc_time  () );
  }
  for (int i_pl = 0; i_pl < N_PL; i_pl++) delete hv[i_pl];
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonH4::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonH4::FindAllMonHist()
{
  ostringstream oss;
  for (int pl = 0; pl < N_PL; pl++) {
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = FindMonHist(oss.str().c_str());
    if (! h1_ele[pl]) return 1;
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = FindMonHist(oss.str().c_str());
    if (! h1_time[pl]) return 1;
  }
  h2_ele  = FindMonHist("h2_ele" );
  h2_time = FindMonHist("h2_time");
  if (! h2_ele || ! h2_time) return 1;
  return 0;
}

int OnlMonH4::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  can0->SetStatus(OnlMonCanvas::OK);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(1, 2);
  TVirtualPad* pad00 = pad0->cd(1); // for 1D histograms
  pad00->Divide(2, 1);
  pad00->cd(1);  h1_ele[0]->Draw();
  pad00->cd(2);  h1_ele[1]->Draw();
  pad0->cd(2); // for 2D histogram
  h2_ele->Draw("colz");
  if (h1_ele[0]->Integral() + h1_ele[1]->Integral() == 0) {
    can0->SetStatus(OnlMonCanvas::WARN);
    can0->AddMessage("Empty.");
  }

  UtilHist::AutoSetRange (h1_time[0]);
  UtilHist::AutoSetRange (h1_time[1]);
  UtilHist::AutoSetRangeX((TH2*)h2_time);
  UtilHist::AutoSetRangeY((TH2*)h2_time);
  OnlMonCanvas* can1 = GetCanvas(1);
  can1->SetStatus(OnlMonCanvas::OK);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(1, 2);
  TVirtualPad* pad10 = pad1->cd(1); // for 1D histograms
  pad10->Divide(2, 1);
  pad10->cd(1);  h1_time[0]->Draw();
  pad10->cd(2);  h1_time[1]->Draw();
  pad1->cd(2); // for 2D histogram
  h2_time->Draw("colz");
  if (h1_time[0]->Integral() + h1_time[1]->Integral() == 0) {
    can1->SetStatus(OnlMonCanvas::WARN);
    can1->AddMessage("Empty.");
  }

  return 0;
}
