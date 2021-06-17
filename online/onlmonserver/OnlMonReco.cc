/// OnlMonReco.C
#include <sstream>
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
//#include <geom_svc/CalibParamInTimeTaiwan.h>
#include <UtilAna/UtilHist.h>
#include "OnlMonReco.h"

#define MUON_MASS 0.105658

using namespace std;

OnlMonReco::OnlMonReco()
{
  NumCanvases(2);
	Title("Reco Mon");
}

int OnlMonReco::InitOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::InitRunOnlMon(PHCompositeNode* topNode)
{
  h1_rec_stats = new TH1D("h1_rec_stats", ";Rec. Stats; ", 7, -6.5, 0.5);
  RegisterHist(h1_rec_stats);

  h1_sgmu_pt = new TH1D("h1_sgmu_pt", ";pt; ", 10, 0, 5);
  RegisterHist(h1_sgmu_pt);

  h1_dimu_mass = new TH1D("h1_dimu_mass", ";mass; ", 20, 0, 10);
  RegisterHist(h1_dimu_mass);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::ProcessEventOnlMon(PHCompositeNode* topNode)
{
	std::cout << "OnlMonReco::ProcessEventOnlMon: " << std::endl;
  SRecEvent* rec_event = findNode::getClass<SRecEvent>(topNode, "SRecEvent");

  if (!rec_event) return Fun4AllReturnCodes::ABORTEVENT;
	std::cout << "getEventID: " << rec_event->getEventID() << std::endl;

	h1_rec_stats->Fill(rec_event->getRecStatus());

  for (int itrack0=0; itrack0<rec_event->getNTracks(); ++itrack0) {
    SRecTrack track0 = rec_event->getTrack(itrack0);
    TVector3 mom0 = track0.getMomentumVecSt1();
		int charge0 = track0.getCharge();
    h1_sgmu_pt->Fill(mom0.Pt());
		for (int itrack1=itrack0+1; itrack1<rec_event->getNTracks(); ++itrack1) {
      SRecTrack track1 = rec_event->getTrack(itrack1);
		  int charge1 = track1.getCharge();
			if(charge0*charge1>0) continue;
      TVector3 mom1 = track1.getMomentumVecSt1();
			TLorentzVector mom40;
			TLorentzVector mom41;
			mom40.SetXYZM(mom0.Px(),mom0.Py(),mom0.Pz(),MUON_MASS);
			mom41.SetXYZM(mom1.Px(),mom1.Py(),mom1.Pz(),MUON_MASS);
			TLorentzVector gamma = mom40 + mom41;
			h1_dimu_mass->Fill(gamma.M());

		}
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::EndOnlMon(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonReco::FindAllMonHist()
{
  h1_rec_stats = FindMonHist("h1_rec_stats");
  if (!h1_rec_stats) return 1;

  h1_sgmu_pt = FindMonHist("h1_sgmu_pt");
  if (!h1_sgmu_pt) return 1;

  h1_dimu_mass = FindMonHist("h1_dimu_mass");
  if (!h1_dimu_mass) return 1;

  return 0;
}

int OnlMonReco::DrawMonitor()
{
  OnlMonCanvas* can0 = GetCanvas(0);
  TPad* pad0 = can0->GetMainPad();
  pad0->Divide(1, 1); // to be divided more when more plots are available.

  TVirtualPad* pad01 = pad0->cd(1);
  pad01->SetGrid();
  h1_rec_stats->Draw();
  can0->AddMessage("OK");
  can0->SetStatus(OnlMonCanvas::OK);

  OnlMonCanvas* can1 = GetCanvas(1);
  TPad* pad1 = can1->GetMainPad();
  pad1->Divide(1, 2);

  TVirtualPad* pad11 = pad1->cd(1);
  pad11->SetGrid();
  h1_sgmu_pt->Draw();

  TVirtualPad* pad12 = pad1->cd(2);
  pad12->SetGrid();
  h1_dimu_mass->Draw();
  can1->AddMessage("OK");
  can1->SetStatus(OnlMonCanvas::OK);

  return 0;
}
