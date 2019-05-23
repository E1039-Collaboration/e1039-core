/// OnlMonReco.C
#include <iomanip>
#include <TH1D.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <ktracker/SRecEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
//#include <chan_map/CalibParamInTimeTaiwan.h>
#include "OnlMonServer.h"
#include "OnlMonReco.h"
#include "UtilHist.h"
using namespace std;

OnlMonReco::OnlMonReco()
{
  NumCanvases(2);
}

int OnlMonReco::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::InitRunOnlMon(PHCompositeNode* topNode)
{
  GeomSvc* geom = GeomSvc::instance();

  h1_pt = new TH1D("h1_pt", ";pt; ", 10, 0, 5);
  RegisterHist(h1_pt);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  SRecEvent* rec_event = findNode::getClass<SRecEvent>(topNode, "SRecEvent");

  if (!rec_event) return Fun4AllReturnCodes::ABORTEVENT;

  for (int itrack=0; itrack<rec_event->getNTracks(); ++itrack) {
    SRecTrack track = rec_event->getTrack(itrack);
    TVector3 mom = track.getMomentumVecSt1();
    h1_pt->Fill(mom.Pt());
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::FindAllMonHist()
{
  h1_pt = (TH1*)FindMonObj("h1_pt");
  if (!h1_pt) return 1;

  return 0;
}

int OnlMonReco::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->SetGrid();
  pad0->Divide(2, 1);
  pad0->cd(1);
  h1_pt->Draw();
  can0->AddMessage("OK");
  can0->SetStatus(OnlMonCanvas::OK);

  return 0;
}
