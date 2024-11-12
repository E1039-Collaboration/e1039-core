#include "SQVertexing.h"

#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHFieldUtility.h>
#include <phgeom/PHGeomUtility.h>

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
  legacyContainer_in(false),
  legacyContainer_out(false),
  enableSingleRetracking(false),
  gfield(nullptr),
  geom_file_name(""),
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

  ret = InitField(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = InitGeom(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::process_event(PHCompositeNode* topNode)
{
  if(legacyContainer_out) recEvent->clearDimuons();

  std::vector<int> trackIDs1;
  std::vector<int> trackIDs2;
  int nTracks = legacyContainer_in ? recEvent->getNTracks() : recTrackVec->size();
  if(Verbosity() > 10) std::cout << "SQVertexing::process_event():  nTracks = " << nTracks << std::endl;
  for(int i = 0; i < nTracks; ++i)
  {
    SRecTrack* recTrack = legacyContainer_in ? &(recEvent->getTrack(i)) : dynamic_cast<SRecTrack*>(recTrackVec->at(i));
    if(!recTrack->isKalmanFitted()) continue;

    if(enableSingleRetracking)
    {
      TVector3 pos, mom;
      double chi2 = refitTrkToVtx(recTrack, Z_TARGET, &pos, &mom);
      if(chi2 > 0)
      {
        recTrack->setChisqTarget(chi2);
        recTrack->setTargetPos(pos);
        recTrack->setTargetMom(mom);
      }
    }

    if(recTrack->getCharge() == charge1) trackIDs1.push_back(i);
    if(recTrack->getCharge() == charge2) trackIDs2.push_back(i);
   
  }
  if(trackIDs1.empty() || trackIDs2.empty()) return Fun4AllReturnCodes::EVENT_OK;
  if(Verbosity() > 10) std::cout << "  N of trackIDs1 & trackIDs1 = " << trackIDs1.size() << " & " << trackIDs2.size() << std::endl;
  
  for(int i = 0; i < trackIDs1.size(); ++i)
  {
    for(int j = charge1 == charge2 ? i+1 : 0; j < trackIDs2.size(); ++j)
    {
      //A protection, probably not really needed
      if(trackIDs1[i] == trackIDs2[j]) continue;

      SRecTrack* recTrack1 = legacyContainer_in ? &(recEvent->getTrack(trackIDs1[i])) : dynamic_cast<SRecTrack*>(recTrackVec->at(trackIDs1[i]));
      SRecTrack* recTrack2 = legacyContainer_in ? &(recEvent->getTrack(trackIDs2[j])) : dynamic_cast<SRecTrack*>(recTrackVec->at(trackIDs2[j]));

      SRecDimuon recDimuon;
      if(processOneDimuon(recTrack1, recTrack2, recDimuon))
      {
        //recDimuon.set_rec_dimuon_id(legacyContainer ? recEvent->getNDimuons() : recDimuonVec->size());
        recDimuon.set_track_id_pos(trackIDs1[i]);
        recDimuon.set_track_id_neg(trackIDs2[j]);

        if(legacyContainer_out)
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
  if(legacyContainer_in || legacyContainer_out)
  {
    recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
    if(!recEvent)
    {
      std::cerr << Name() << ": failed finding SRecEvent node, abort." << std::endl;
      return Fun4AllReturnCodes::ABORTRUN;
    }
  }
  else
  {
    recTrackVec = findNode::getClass<SQTrackVector>(topNode, "SQRecTrackVector");
    if(!recTrackVec)
    {
      std::cerr << Name() << ": failed finding SQRecTrackVector node, abort." << std::endl;
      return Fun4AllReturnCodes::ABORTRUN;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::MakeNodes(PHCompositeNode* topNode)
{
  if(!legacyContainer_out)
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
  double chi2 = track.swimToVertex(z, pos, mom);
  return chi2;
}

double SQVertexing::refitTrkToVtx(SQGenFit::GFTrack& track, double z, TVector3* pos, TVector3* mom)
{
  if(Verbosity() > 20) std::cout << "SQVertexing::refitTrkToVtx(): z = " << z << ", pos = (" << pos->X() << ", " << pos->Y() << ", " << pos->Z() << "), mom = (" << mom->X() << ", " << mom->Y() << ", " << mom->Z() << ")" << std::endl;
  gfield->setOffset(0.);

  double z_offset_prev = 1.E9;
  double z_offset_curr = 0.;
  double chi2 = 0.;
  const int n_iter_max = 100;
  int n_iter = 0;
  while(fabs(z_offset_curr - z_offset_prev) > 0.1)
  {
    chi2 = swimTrackToVertex(track, z, pos, mom);

    //update scatter plane location
    z_offset_prev = z_offset_curr;
    z_offset_curr = calcZsclp(mom->Mag());
  
    gfield->setOffset(-z_offset_curr);
    if(Verbosity() > 20) std::cout << "  i_iter = " << n_iter << ": p = " << mom->Mag() << ", dz = " << z_offset_curr << " - " << z_offset_prev << " = " << z_offset_curr - z_offset_prev << std::endl;
    if (++n_iter == n_iter_max) {
      std::cout << "!WARNING!  SQVertexing::refitTrkToVtx():  Give up the iteration." << std::endl;
      break;
    }
  }
  if(Verbosity() > 10) std::cout << "  n_iter = " << n_iter << std::endl;
  
  //re-adjust FMag bend plane here
  gfield->setOffset(0.);

  return chi2;
}

double SQVertexing::refitTrkToVtx(SRecTrack* track, double z, TVector3* pos, TVector3* mom)
{
  SQGenFit::GFTrack gtrk(*track);
  return refitTrkToVtx(gtrk, z, pos, mom);
}

double SQVertexing::calcZsclp(double p)
{
  if(p < 5.) p = 5.;
  else if(p > 120.) p = 120.;
  
  return 301.84 - 1.27137*p + 0.0218294*p*p - 0.000170711*p*p*p + 4.94683e-07*p*p*p*p - 271.;
}

bool SQVertexing::processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon)
{
  if(Verbosity() > 10) std::cout << "  SQVertexing::processOneDimuon(): " << std::endl;
  
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
  //TODO: consider using addjustable bend-plane for vertex test as well
  TVector3 pos, mom;
  dimuon.chisq_kf = swimTrackToVertex(gtrk1, z_vtx, &pos, &mom);
  dimuon.p_pos.SetVectM(mom, M_MU);
  dimuon.vtx_pos = pos;

  dimuon.chisq_kf += swimTrackToVertex(gtrk2, z_vtx, &pos, &mom);
  dimuon.p_neg.SetVectM(mom, M_MU);
  dimuon.vtx_neg = pos;

  //Test target hypothesis
  refitTrkToVtx(gtrk1, Z_TARGET, &pos, &mom);
  dimuon.p_pos_target.SetVectM(mom, M_MU);
  refitTrkToVtx(gtrk2, Z_TARGET, &pos, &mom);
  dimuon.p_neg_target.SetVectM(mom, M_MU);

  //Test dump hypothesis
  //TODO: consider using addjustable bend-plane for dump test as well
  swimTrackToVertex(gtrk1, Z_DUMP, &pos, &mom);
  dimuon.p_pos_dump.SetVectM(mom, M_MU);
  swimTrackToVertex(gtrk2, Z_DUMP, &pos, &mom);
  dimuon.p_neg_dump.SetVectM(mom, M_MU);
  
  //Flip the sign of Px of 'positive' muon if processing like-sign muons
  if(charge1 == charge2)
  {
    dimuon.p_pos.SetPx(-dimuon.p_pos.Px());
    dimuon.p_pos_target.SetPx(-dimuon.p_pos_target.Px());
    dimuon.p_pos_dump.SetPx(-dimuon.p_pos_dump.Px());
  }

  return true;
}

double SQVertexing::findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2)
{
  //TODO: consider using addjustable bend-plane for vertex finding as well
  double stepsize[3] = {25., 5., 1.};

  double z_min = 300.;
  double chi2_min = 1.E9;
  for(int i = 0; i < 3; ++i)
  {
    double z_start, z_end;
    if(i == 0)
    {
      z_start = z_min;
      z_end   = Z_UPSTREAM;
    }
    else
    {
      z_start = z_min + stepsize[i-1];
      z_end   = z_min - stepsize[i-1];
    }

    for(double z = z_start; z > z_end; z = z - stepsize[i])
    {
      double chi2_1 = swimTrackToVertex(track1, z);
      double chi2_2 = swimTrackToVertex(track2, z);
      double chi2 = chi2_1 > 0 && chi2_2 > 0 ? chi2_1 + chi2_2 : 1.E9;
      if(chi2 < chi2_min)
      {
        z_min = z;
        chi2_min = chi2;
      }
    }
  }

  return z_min;
}

int SQVertexing::InitField(PHCompositeNode* topNode)
{
  try
  {
    gfield = dynamic_cast<SQGenFit::GFField*>(genfit::FieldManager::getInstance()->getField());
  }
  catch(const std::exception& e)
  {
    std::cout << "SQVertexing::InitGeom - Caught an exception " << std::endl;
    gfield = nullptr;
  }
  
  if(gfield == nullptr)
  {
    recoConsts* rc = recoConsts::instance();

    std::unique_ptr<PHFieldConfig> default_field_cfg(new PHFieldConfig_v3(rc->get_CharFlag("fMagFile"), rc->get_CharFlag("kMagFile"), rc->get_DoubleFlag("FMAGSTR"), rc->get_DoubleFlag("KMAGSTR"), 5.));
    PHField* phfield = PHFieldUtility::GetFieldMapNode(default_field_cfg.get(), topNode, 0);

    std::cout << "SQVertexing::InitGeom - creating new GenFit field map" << std::endl;
    gfield = new SQGenFit::GFField(phfield);
  }
  else
  {
    std::cout << "SQVertexing::InitGeom - reading existing GenFit field map" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQVertexing::InitGeom(PHCompositeNode* topNode)
{
  if(geom_file_name != "")
  {
    std::cout << "SQVertexing::InitGeom - create geom from " << geom_file_name << std::endl;

    int ret = PHGeomUtility::ImportGeomFile(topNode, geom_file_name);
    if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  }
  else
  {
    std::cout << "SQVertexing::InitGeom - rely on existing TGeo geometry" << std::endl;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}
