#include "SQTruthVertexing.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <interface_main/SQTrack_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQTrackVector_v1.h>
#include <interface_main/SQDimuonVector_v1.h>

#include "SRecEvent.h"
#include "GFTrack.h"


#include <iostream>
#include <vector>

SQTruthVertexing::SQTruthVertexing(const std::string& name):
  SubsysReco(name),
  legacyContainer(false),
  vtxSmearing(false),
  vtxResolution(35.),
  recEvent(nullptr),
  recTrackVec(nullptr),
  truthTrackVec(nullptr),
  truthDimuonVec(nullptr),
  recDimuonVec(nullptr)
{}

SQTruthVertexing::~SQTruthVertexing()
{}

int SQTruthVertexing::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::InitRun(PHCompositeNode* topNode)
{
  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = MakeNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  if(vtxSmearing) rndm.SetSeed(PHRandomSeed());

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::process_event(PHCompositeNode* topNode)
{
  //start with processing all the rec tracks using its truth vertex - match by momentum at station-1 
  //TODO - in the future may consider using st1 hits instead?
  std::map<int, SRecTrack*> posTracks;
  std::map<int, SRecTrack*> negTracks;
  int nTracks = legacyContainer ? recEvent->getNTracks() : recTrackVec->size();
  for(int i = 0; i < nTracks; ++i)
  {
    SRecTrack* recTrack = legacyContainer ? &(recEvent->getTrack(i)) : (recTrackVec->at(i));
    int truthID = findTruthTrack(recTrack);
    if(truthID == -1) continue;

    SQTrack* truthTrack = truthTrackVec->at(i);
    double z_vtx = truthTrack->get_pos_vtx().Z() + (vtxSmearing ? rndm.Gaus(0., vtxResolution) : 0.);

    if(!swimTrackToVertex(recTrack, z_vtx)) continue;
    if(recTrack->getCharge() > 0)
    {
      posTracks[truthTrack->get_track_id()] = recTrack;
    }
    else
    {
      negTracks[truthTrack->get_track_id()] = recTrack;
    }
  }

  if(posTracks.empty() || negTracks.empty()) return Fun4AllReturnCodes::EVENT_OK;

  //if true dimuons exist, only pair the tracks that are associated with the true dimuon, otherwise produce all possible combinations
  if(truthDimuonVec)
  {
    int nTrueDimuons = truthDimuonVec->size();
    for(int i = 0; i < nTrueDimuons; ++i)
    {
      SQDimuon* trueDimuon = truthDimuonVec->at(i);
      int pid = trueDimuon->get_track_id_pos();
      int mid = trueDimuon->get_track_id_neg();
      if(posTracks.find(pid) == posTracks.end() || negTracks.find(mid) == negTracks.end()) continue;

      SRecDimuon recDimuon;

      if(!legacyContainer)
        recDimuonVec->push_back(&recDimuon);
      else
        recEvent->insertDimuon(recDimuon);
    }
  }
  else
  {
    for(auto ptrk = posTracks.begin(); ptrk != posTracks.end(); ++ptrk)
    {
      for(auto mtrk = negTracks.begin(); mtrk != negTracks.end(); ++mtrk)
      {
        SRecDimuon recDimuon;

        if(!legacyContainer)
          recDimuonVec->push_back(&recDimuon);
        else
          recEvent->insertDimuon(recDimuon);
      }
    }
  }


  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::GetNodes(PHCompositeNode* topNode)
{
  truthTrackVec = findNode::getClass<SQTrackVector>(topNode, "SQTruthTrackVector");
  if(!truthTrackVec)
  {
    std::cerr << Name() << ": failed finding truth track info, abort." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  truthDimuonVec = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if(!truthDimuonVec)
  {
    std::cout << Name() << ": failed finding truth dimuon info, rec dimuon will be for reference only. " << std::endl;
  }

  if(legacyContainer)
  {
    recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
  }
  else
  {
    recTrackVec = findNode::getClass<SRecTrackVector>(topNode, "SRecTrackVector");
  }

  if((!recEvent) && (!recTrackVec))
  {
    std::cerr << Name() << ": failed finding rec track info, abort." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::MakeNodes(PHCompositeNode* topNode)
{
  if(!legacyContainer)
  {
    PHNodeIterator iter(topNode);
    PHCompositeNode* dstNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
    if(!dstNode) 
    {
      std::cerr << Name() << ": cannot locate DST node, abort." << std::endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }

    recDimuonVec = new SRecDimuonVector();
    dstNode->addNode(new PHIODataNode<PHObject>(recDimuonVec, "SRecDimuonVector", "PHObject"));
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQTruthVertexing::findTruthTrack(SRecTrack* recTrack)
{
  return 0;
}

bool SQTruthVertexing::swimTrackToVertex(SRecTrack* track, double z, TVector3* pos, TVector3* mom)
{
  //Basic constants
  TVector3 pU(1., 0., 0.);
  TVector3 pV(0., 1., 0.);
  TVector3 pO(0., 0., z);
  
  TVectorD beamCenter(2);
  beamCenter[0] = 0.; beamCenter[1] = 0.;
  TMatrixDSym beamCov(2);
  beamCov.Zero();
  beamCov(0, 0) = 100.; beamCov(1, 1) = 100.;

  try
  {
    SQGenFit::GFTrack gftrk(*track);

    double len = gftrk.extrapolateToPlane(pO, pU, pV);
    if(fabs(len) > 6000.) throw len;

    if(pos == nullptr)
    {
      TVector3 pos, mom;

      track->setChisqVertex(gftrk.updatePropState(beamCenter, beamCov));
      gftrk.getExtrapPosMom(pos, mom);

      track->setVertexPos(pos);
      track->setVertexMom(mom);
    }
    else
    {
      gftrk.updatePropState(beamCenter, beamCov);
      gftrk.getExtrapPosMom(*pos, *mom);
    }
  }
  catch(genfit::Exception& e)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed vertex @Z=" << track->getVertexPos().Z() << ": " << e.what() << std::endl;
    return false;
  }
  catch(double len)
  {
    std::cerr << __FILE__ << " " << __LINE__ << ": hypo test failed vertex @Z=" << track->getVertexPos().Z() << ": " << len << std::endl;
    return false;
  }

  return true;
}
