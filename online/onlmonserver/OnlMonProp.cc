/// OnlMonProp.C
#include <sstream>
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
#include <UtilAna/UtilSQHit.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonProp.h"
using namespace std;

OnlMonProp::OnlMonProp(const PropType_t type) : m_type(type)
{
  NumCanvases(2);
  switch (m_type) {
  case P1:  Name("OnlMonPropP1" );  Title( "Prop Tube: P1");  break;
  case P2:  Name("OnlMonPropP2" );  Title( "Prop Tube: P2");  break;
  }
}

int OnlMonProp::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonProp::InitRunOnlMon(PHCompositeNode* topNode)
{
  GeomSvc* geom = GeomSvc::instance();
  string name_regex = "";
  switch (m_type) {
  case P1:  name_regex = "^P1";  break;
  case P2:  name_regex = "^P2";  break;
  }
  vector<int> list_det_id = geom->getDetectorIDs(name_regex);
  if (list_det_id.size() == 0) {
    cout << "OnlMonProp::InitRunOnlMon():  Found no ID for '" << name_regex << "'." << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  m_pl0 = list_det_id[0];

  ostringstream oss;
  for (int pl = 0; pl < N_PL; pl++) {
    string name = geom->getDetectorName  (m_pl0 + pl);
    int   n_ele = geom->getPlaneNElements(m_pl0 + pl); 
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;Hit count";
    h1_ele[pl]->SetTitle(oss.str().c_str());

    const double DT = 80/9.0; // 4/9 ns per single count of Taiwan TDC
    const int NT    = 100;
    const double T0 =   0.5*DT;
    const double T1 = 100.5*DT;

    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = new TH1D(oss.str().c_str(), "", NT, T0, T1);

    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[pl]->SetTitle(oss.str().c_str());

    RegisterHist(h1_ele [pl]);
    RegisterHist(h1_time[pl]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonProp::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SQEvent*     evt     = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector* hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!evt || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (int pl = 0; pl < N_PL; pl++) {
    int det_id = pl + m_pl0;
    auto vec = UtilSQHit::FindHitsFast(evt, hit_vec, det_id);
    for (auto it = vec->begin(); it != vec->end(); it++) {
      h1_ele [pl]->Fill((*it)->get_element_id());
      h1_time[pl]->Fill((*it)->get_tdc_time  ());
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonProp::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonProp::FindAllMonHist()
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
  return 0;
}

int OnlMonProp::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(2, N_PL/2); // Assume N_PL is even.
  for (int pl = 0; pl < N_PL; pl++) {
    TVirtualPad* pad0i = pad0->cd(pl+1);
    pad0i->SetGrid();
    h1_ele[pl]->Draw();
  }
  //can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(2, N_PL/2); // Assume N_PL is even.
  for (int pl = 0; pl < N_PL; pl++) {
    TVirtualPad* pad1i = pad1->cd(pl+1);
    pad1i->SetGrid();
    UtilHist::AutoSetRange(h1_time[pl]);
    h1_time[pl]->Draw();
  }
  //can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}
