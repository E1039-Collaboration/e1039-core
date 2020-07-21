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
class SRecTrackVector;
class SRecDimuonVector;

class SQTruthVertexing: public SubsysReco
{
public:
  SQTruthVertexing(const std::string& name = "SQTruthVertexing");
  ~SQTruthVertexing();

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void set_enable_legacy_container(const bool enable = true) { legacyContainer = enable; }
  void set_enable_vtx_smearing(const bool enable = true) { vtxSmearing = enable; };
  void set_vtx_resolution(const double r) { vtxResolution = r;}

private:
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode* topNode);

  int findTruthTrack(SRecTrack* recTrack);
  bool swimTrackToVertex(SRecTrack* track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);

  TRandom1 rndm;

  bool legacyContainer;
  bool vtxSmearing;
  double vtxResolution;

  SRecEvent* recEvent;
  SRecTrackVector* recTrackVec;
  SQTrackVector*  truthTrackVec;
  SQDimuonVector* truthDimuonVec;

  SRecDimuonVector* recDimuonVec;
};

#endif