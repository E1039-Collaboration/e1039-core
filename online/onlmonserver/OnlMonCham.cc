/// OnlMonCham.C
#include <iomanip>
#include <TH1D.h>
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
#include "OnlMonCham.h"
using namespace std;

OnlMonCham::OnlMonCham(const ChamType_t type, const std::string& name) : m_type(type), m_pl0(0), OnlMonClient(name)
{
  SetNumCanvases(2);
  switch (m_type) {
  case D0 :  m_pl0 =  1;  Name("OnlMonChamD0" );  break;
  case D1 :  m_pl0 =  7;  Name("OnlMonChamD1" );  break;
  case D2 :  m_pl0 = 13;  Name("OnlMonChamD2" );  break;
  case D3p:  m_pl0 = 19;  Name("OnlMonChamD3p");  break;
  case D3m:  m_pl0 = 25;  Name("OnlMonChamD3m");  break;
  }
}

int OnlMonCham::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonCham::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  cout << "SQRun: " << run_header->get_run_id() << " " << run_header->get_spill_count() << endl;

  GeomSvc* geom = GeomSvc::instance();
  CalibParamInTimeTaiwan calib;
  calib.SetMapIDbyDB(run_header->get_run_id());
  calib.ReadFromDB();

  Fun4AllHistoManager* hm = new Fun4AllHistoManager(Name());
  OnlMonServer::instance()->registerHistoManager(hm);

  ostringstream oss;
  for (int pl = 0; pl < N_PL; pl++) {
    string name = geom->getDetectorName(m_pl0 + pl);
    int n_ele = geom->getPlaneNElements(m_pl0 + pl); 
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = new TH1D(oss.str().c_str(), "", n_ele, 0.5, n_ele+0.5);
    oss.str("");
    oss << name << ";Element ID;Hit count";
    h1_ele[pl]->SetTitle(oss.str().c_str());

    double center, width;
    calib.Find(m_pl0 + pl, 1, center, width);
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = new TH1D(oss.str().c_str(), "", 100, center-width/2, center+width/2);
    oss.str("");
    oss << name << ";tdcTime;Hit count";
    h1_time[pl]->SetTitle(oss.str().c_str());

    hm->registerHisto(h1_ele [pl]);
    hm->registerHisto(h1_time[pl]);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonCham::process_event(PHCompositeNode* topNode)
{
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!event_header || !hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    int pl = (*it)->get_detector_id() - m_pl0;
    if (pl < 0 || pl >= N_PL) continue;
    h1_ele [pl]->Fill((*it)->get_element_id());
    h1_time[pl]->Fill((*it)->get_tdc_time  ());
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonCham::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonCham::DrawMonitor()
{
  ostringstream oss;
  for (int pl = 0; pl < N_PL; pl++) {
    oss.str("");
    oss << "h1_ele_" << pl;
    h1_ele[pl] = (TH1*)FindMonObj(oss.str().c_str());
    oss.str("");
    oss << "h1_time_" << pl;
    h1_time[pl] = (TH1*)FindMonObj(oss.str().c_str());
  }

  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(2, 3);
  for (int pl = 0; pl < N_PL; pl++) {
    pad0->cd(pl+1);
    if (h1_ele[pl]->Integral() > 1000) gPad->SetLogy();
    h1_ele[pl]->Draw();
  }
  can0->AddMessage("Always Okay ;^D");
  can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->SetGrid();
  pad1->Divide(2, 3);
  for (int pl = 0; pl < N_PL; pl++) {
    pad1->cd(pl+1);
    if (h1_time[pl]->Integral() > 1000) gPad->SetLogy();
    h1_time[pl]->Draw();
  }
  can1->AddMessage("Always Okay ;^D");
  can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}
