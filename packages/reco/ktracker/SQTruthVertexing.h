#ifndef _SQTRUTHVERTEXING_H
#define _SQTRUTHVERTEXING_H

#include <fun4all/SubsysReco.h>
#include <TRandom1.h>

class TVector3;
class SQTrack;
class SQTrackVector;
class SQDimuonVector;
class SRecEvent;
class SRecTrack;
class SRecDimuon;

/// An SubsysReco module to create create dimuons based on the truth vertex information
/**
 * This module adds the following information depending on the container type of the reconstruction info
 *  - SRecDimuon based on either the truth dimuon info, or random compbination of all available dimuon paris
 *  - SRecTrack, if is matched to a true track, its single muon vertex information (pos/mom/chi2) will be 
 *    updated with truth vertex information
 * 
 * In all reconstruction-related modules, two different container mode is supported:
 *  - legacy mode: all the reconstructed tracks (SRecTrack) and dimuons (SRecDimuon) are stored inside a
 *    SRecEvent node which internally maintains vectors of all tracks and dimuons
 *  - Fun4All-style mode: container node SQTrackVector and SQDimuonVector are used to store the 
 *    reconstructed tracks and dimuons. The access interface is the same as other fun4all containers like SQHitVector
 *
 * Since this module relies on the existence of SQTrack and its correlation with SRecTrack, it needs to be added after 
 * TruthNodeMaker otherwise it will throw ABORT in InitRun().
 * 
 * Typical usage looks like this:
 * 
 *  SQTruthVertexing* truthvtx = new SQTruthVertexing();
 *  truthvtx->set_legacy_rec_container(false);    // set the rec info container type like mentioned above, default is true
 *  truthvtx->set_vtx_smearing(50.);              // smear the truth z_vertex to mimic resolution effect, default is 0.
 *  se->registerSubsystem(truthvtx);
 */

class SQTruthVertexing: public SubsysReco
{
public:
  SQTruthVertexing(const std::string& name = "SQTruthVertexing");
  ~SQTruthVertexing();

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void set_legacy_rec_container(const bool enable = true) { legacyContainer = enable; }
  void set_vtx_smearing(const double r) { vtxSmearing = r; }

private:
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode* topNode);

  bool buildRecDimuon(double z_vtx, SRecTrack* posTrack, SRecTrack* negTrack, SRecDimuon* dimuon);
  double swimTrackToVertex(SRecTrack* track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);

  TRandom1 rndm;

  bool legacyContainer;
  double vtxSmearing;

  SRecEvent*      recEvent;
  SQTrackVector*  recTrackVec;
  SQTrackVector*  truthTrackVec;
  SQDimuonVector* truthDimuonVec;

  SQDimuonVector* recDimuonVec;
};

#endif