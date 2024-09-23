#include "SQVertexing.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <phool/recoConsts.h>
#include <interface_main/SQTrack_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQTrackVector_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <GenFit/FieldManager.h>

#include "SRecEvent.h"
#include "GFTrack.h"

#include <iostream>
#include <vector>

namespace
{
  //static flag to indicate the initialized has been done
  static bool inited = false;

  static double Z_TARGET;
  static double Z_DUMP;
  static double Z_UPSTREAM;

  static double X_BEAM;
  static double Y_BEAM;
  static double SIGX_BEAM;
  static double SIGY_BEAM;

  //initialize global variables
  void initGlobalVariables()
  {
    if(!inited) 
    {
      inited = true;

      recoConsts* rc = recoConsts::instance();
      Z_TARGET   = rc->get_DoubleFlag("Z_TARGET");
      Z_DUMP     = rc->get_DoubleFlag("Z_DUMP");
      Z_UPSTREAM = rc->get_DoubleFlag("Z_UPSTREAM");

      X_BEAM    = rc->get_DoubleFlag("X_BEAM");
      Y_BEAM    = rc->get_DoubleFlag("Y_BEAM");
      SIGX_BEAM = rc->get_DoubleFlag("SIGX_BEAM");
      SIGY_BEAM = rc->get_DoubleFlag("SIGY_BEAM");
    }
  }
};

SQVertexing::SQVertexing(const std::string& name, int sign1, int sign2):
  SubsysReco(name),
  legacyContainer(false),
  gfield(nullptr),
  recEvent(nullptr),
  recTrackVec(nullptr),
  recDimuonVec(nullptr),
  charge1(sign1),
  charge2(sign2)
{}

SQVertexing::~SQVertexing()
{}

int SQVertexing::Init(PHCompositeNode* topNode)
{
  initGlobalVariables();
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::InitRun(PHCompositeNode* topNode)
{
  int ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = MakeNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  gfield = dynamic_cast<SQGenFit::GFField*>(genfit::FieldManager::getInstance()->getField());

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::process_event(PHCompositeNode* topNode)
{
  std::vector<int> trackIDs1;
  std::vector<int> trackIDs2;
  int nTracks = legacyContainer ? recEvent->getNTracks() : recTrackVec->size();
  for(int i = 0; i < nTracks; ++i)
  {
    SRecTrack* recTrack = legacyContainer ? &(recEvent->getTrack(i)) : dynamic_cast<SRecTrack*>(recTrackVec->at(i));

    if(recTrack->getCharge() == charge1) trackIDs1.push_back(i);
    if(recTrack->getCharge() == charge2) trackIDs2.push_back(i);
   
  }
  if(trackIDs1.empty() || trackIDs2.empty()) return Fun4AllReturnCodes::EVENT_OK;

  for(int i = 0; i < trackIDs1.size(); ++i)
  {
    for(int j = charge1 == charge2 ? i+1 : 0; j < trackIDs2.size(); ++j)
    {
      //A protection, probably not really needed
      if(trackIDs1[i] == trackIDs2[j]) continue;

      SRecTrack* recTrack1 = legacyContainer ? &(recEvent->getTrack(trackIDs1[i])) : dynamic_cast<SRecTrack*>(recTrackVec->at(trackIDs1[i]));
      SRecTrack* recTrack2 = legacyContainer ? &(recEvent->getTrack(trackIDs2[j])) : dynamic_cast<SRecTrack*>(recTrackVec->at(trackIDs2[j]));

      SRecDimuon recDimuon;
      if(processOneDimuon(recTrack1, recTrack2, recDimuon))
      {
        //recDimuon.set_rec_dimuon_id(legacyContainer ? recEvent->getNDimuons() : recDimuonVec->size());
        recDimuon.set_track_id_pos(trackIDs1[i]);
        recDimuon.set_track_id_neg(trackIDs2[j]);

        if(legacyContainer)
          recEvent->insertDimuon(recDimuon);
        else
          recDimuonVec->push_back(&recDimuon);
      }
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::GetNodes(PHCompositeNode* topNode)
{
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

int SQVertexing::MakeNodes(PHCompositeNode* topNode)
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
    TString dimuonVecName = TString::Format("SQRecDimuonVector_%s%s", (charge1 == 1 ? "P" : "M"), (charge2 == 1 ? "P" : "M"));
    dstNode->addNode(new PHIODataNode<PHObject>(recDimuonVec, dimuonVecName.Data(), "PHObject"));
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

double SQVertexing::swimTrackToVertex(SQGenFit::GFTrack& track, double z, TVector3* pos, TVector3* mom)
{
  //TODO: adjust FMag bend plan here
  //gfield->setOffset(xxx);

  double chi2 = track.swimToVertex(z, pos, mom);
  if(chi2 < 0.) return chi2;

  //TODO: re-adjust FMag bend plane here
  //gfield->setOffset(-xxx);

  return chi2;
}

bool SQVertexing::processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon)
{
  //Pre-calculated variables
  dimuon.proj_target_pos = track1->getTargetPos();
  dimuon.proj_dump_pos   = track1->getDumpPos();
  dimuon.proj_target_neg = track2->getTargetPos();
  dimuon.proj_dump_neg   = track2->getDumpPos();
  dimuon.chisq_target    = track1->getChisqTarget() + track2->getChisqTarget();
  dimuon.chisq_dump      = track1->getChisqDump() + track2->getChisqDump();
  dimuon.chisq_upstream  = track1->getChisqUpstream() + track2->getChisqUpstream();
  dimuon.chisq_single    = 0.;  //no longer used
  dimuon.chisq_vx        = 0.;  //no longer used

  //Vertexing part
  SQGenFit::GFTrack gtrk1(*track1);
  SQGenFit::GFTrack gtrk2(*track2);
  double z_vtx = findDimuonZVertex(dimuon, gtrk1, gtrk2);
  dimuon.vtx.SetXYZ(0., 0., z_vtx);

  //Vertex info based on the dimuon vertex
  TVector3 pos, mom;
  dimuon.chisq_kf = swimTrackToVertex(gtrk1, z_vtx, &pos, &mom);
  dimuon.p_pos.SetVectM(mom, M_MU);
  dimuon.vtx_pos = pos;

  dimuon.chisq_kf += swimTrackToVertex(gtrk2, z_vtx, &pos, &mom);
  dimuon.p_neg.SetVectM(mom, M_MU);
  dimuon.vtx_neg = pos;

  //Test target hypothesis
  swimTrackToVertex(gtrk1, Z_TARGET, &pos, &mom);
  dimuon.p_pos_target.SetVectM(mom, M_MU);
  swimTrackToVertex(gtrk2, Z_TARGET, &pos, &mom);
  dimuon.p_neg_target.SetVectM(mom, M_MU);

  //Test dump hypothesis
  swimTrackToVertex(gtrk1, Z_DUMP, &pos, &mom);
  dimuon.p_pos_dump.SetVectM(mom, M_MU);
  swimTrackToVertex(gtrk2, Z_DUMP, &pos, &mom);
  dimuon.p_neg_dump.SetVectM(mom, M_MU);
  
  dimuon.calcVariables();
  return true;
}

double SQVertexing::findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2)
{
  double stepsize[3] = {25., 5., 1.};

  double z_min = 200.;
  double chi2_min = 1.E9;
  for(int i = 0; i < 3; ++i)
  {
    double z_start, z_end;
    if(i == 0)
    {
      z_start = 200.;
      z_end   = Z_UPSTREAM;
    }
    else
    {
      z_start = z_min + stepsize[i-1];
      z_end   = z_min - stepsize[i-1];
    }

    for(double z = z_start; z > z_end; z = z - stepsize[i])
    {
      double chi2 = swimTrackToVertex(track1, z) + swimTrackToVertex(track2, z);
      if(chi2 < chi2_min)
      {
        z_min = z;
        chi2_min = chi2;
      }
    }
  }

  return z_min;
}
