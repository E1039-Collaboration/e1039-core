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
  vtxSmearing(-1.),
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
  std::map<int, SRecTrack*> posTracks;
  std::map<int, SRecTrack*> negTracks;
  int nTracks = truthTrackVec->size();
  int nRecTracks = legacyContainer ? recEvent->getNTracks() : recTrackVec->size();
  for(int i = 0; i < nTracks; ++i)
  {
    SQTrack* trk = truthTrackVec->at(i);
    int recTrackIdx = trk->get_rec_track_id();
    if(recTrackIdx < 0 || recTrackIdx >= nRecTracks) continue;

    SRecTrack* recTrack = legacyContainer ? &(recEvent->getTrack(recTrackIdx)) : dynamic_cast<SRecTrack*>(recTrackVec->at(recTrackIdx));
    double z_vtx = trk->get_pos_vtx().Z() + (vtxSmearing>0. ? rndm.Gaus(0., vtxSmearing) : 0.);

    if(swimTrackToVertex(recTrack, z_vtx) < 0.) continue;
    if(recTrack->getCharge() > 0)
    {
      posTracks[trk->get_track_id()] = recTrack;
    }
    else
    {
      negTracks[trk->get_track_id()] = recTrack;
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
      double z_vtx = trueDimuon->get_pos().Z() + (vtxSmearing>0. ? rndm.Gaus(0., vtxSmearing) : 0.);
      if(!buildRecDimuon(z_vtx, posTracks[pid], negTracks[mid], &recDimuon)) continue;
      recDimuon.trackID_pos = pid;
      recDimuon.trackID_neg = mid;

      trueDimuon->set_rec_dimuon_id(legacyContainer ? recEvent->getNDimuons() : recDimuonVec->size());
      if(!legacyContainer)
        recDimuonVec->push_back(&recDimuon);
      else
        recEvent->insertDimuon(recDimuon);
    }
  }
  // else
  // {
  //   for(auto ptrk = posTracks.begin(); ptrk != posTracks.end(); ++ptrk)
  //   {
  //     double z_pos = ptrk->second->getVertexPos().Z();
  //     for(auto mtrk = negTracks.begin(); mtrk != negTracks.end(); ++mtrk)
  //     {
  //       double z_neg = mtrk->second->getVertexPos().Z();
  //       if(fabs(z_pos - z_neg) > 100.) continue;

  //       double z_vtx = 0.5*(z_pos + z_neg) + (vtxSmearing>0. ? rndm.Gaus(0., vtxSmearing) : 0.);
  //       SRecDimuon recDimuon;
  //       if(!buildRecDimuon(z_vtx, ptrk->second, mtrk->second, &recDimuon)) continue;
  //       recDimuon.trackID_pos = ptrk->first;
  //       recDimuon.trackID_neg = mtrk->first;

  //       if(!legacyContainer)
  //         recDimuonVec->push_back(&recDimuon);
  //       else
  //         recEvent->insertDimuon(recDimuon);
  //     }
  //   }
  // }

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
    recTrackVec = findNode::getClass<SQTrackVector>(topNode, "SQRecTrackVector");
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

    recDimuonVec = new SQDimuonVector_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(recDimuonVec, "SQRecDimuonVector", "PHObject"));
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

double SQTruthVertexing::swimTrackToVertex(SRecTrack* track, double z, TVector3* pos, TVector3* mom)
{
  SQGenFit::GFTrack gftrk(*track);

  TVector3 p, m;
  double chi2 = gftrk.swimToVertex(z, &p, &m);
  if(chi2 < 0.) return chi2;

  if(pos == nullptr)
  {
    track->setChisqVertex(chi2);
    track->setVertexPos(p);
    track->setVertexMom(m);
  }
  else
  {
    pos->SetXYZ(p.X(), p.Y(), p.Z());
    mom->SetXYZ(m.X(), m.Y(), m.Z());
  }

  return chi2;
}

bool SQTruthVertexing::buildRecDimuon(double z_vtx, SRecTrack* posTrack, SRecTrack* negTrack, SRecDimuon* dimuon)
{
  TVector3 p_mom, p_pos = posTrack->getVertexPos();
  double p_chi2;
  if(fabs(p_pos.Z() - z_vtx) > 1.)
  {
    p_chi2 = swimTrackToVertex(posTrack, z_vtx, &p_pos, &p_mom);
    if(p_chi2 < 0.) return false;
  }
  else
  {
    p_mom = posTrack->getVertexMom();
    p_chi2 = posTrack->getChisqVertex();
  }

  TVector3 m_mom, m_pos = negTrack->getVertexPos();
  double m_chi2;
  if(fabs(m_pos.Z() - z_vtx) > 1.)
  {
    m_chi2 = swimTrackToVertex(negTrack, z_vtx, &p_pos, &p_mom);
    if(m_chi2 < 0.) return false;
  }
  else
  {
    m_mom = negTrack->getVertexMom();
    m_chi2 = negTrack->getChisqVertex();
  }

  dimuon->p_pos.SetVectM(p_mom, M_MU);
  dimuon->p_neg.SetVectM(m_mom, M_MU);
  dimuon->chisq_kf = p_chi2 + m_chi2;
  dimuon->vtx.SetXYZ(0., 0., z_vtx);
  dimuon->vtx_pos = p_pos;
  dimuon->vtx_neg = m_pos;
  dimuon->proj_target_pos = posTrack->getTargetPos();
  dimuon->proj_dump_pos = posTrack->getDumpPos();
  dimuon->proj_target_neg = negTrack->getTargetPos();
  dimuon->proj_dump_neg = negTrack->getDumpPos();
  dimuon->chisq_target = posTrack->getChisqTarget() + negTrack->getChisqTarget();
  dimuon->chisq_dump = posTrack->getChisqDump() + negTrack->getChisqDump();
  dimuon->chisq_upstream = posTrack->getChisqUpstream() + negTrack->getChisqUpstream();
  dimuon->chisq_single = 0.;
  dimuon->chisq_vx = 0.;
  dimuon->calcVariables();

  return true;
}
