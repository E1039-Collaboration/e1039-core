#ifndef _SQVERTEXING_H
#define _SQVERTEXING_H

#include <fun4all/SubsysReco.h>

#include "GFField.h"
#include "GFTrack.h"

class TString;
class TVector3;
class SQTrack;
class SQTrackVector;
class SQDimuonVector;
class SRecEvent;
class SRecTrack;
class SRecDimuon;

class SQVertexing: public SubsysReco
{
public:
  SQVertexing(const std::string& name = "SQVertexing", int sign1 = 1, int sign2 = -1);
  ~SQVertexing();

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void set_legacy_rec_container(const bool enable = true)  { legacyContainer = enable; }
  void set_single_retracking(const bool enable = true)     { enableSingleRetracking = true; }

  void set_geom_file_name(const std::string& geomFileName) { geom_file_name = geomFileName; }

private:
  int InitField(PHCompositeNode* topNode);
  int InitGeom(PHCompositeNode*  topNode);
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode*  topNode);

  double swimTrackToVertex(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double refitTrkToVtx(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double refitTrkToVtx(SRecTrack*         track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2);
  double calcZsclp(double p);
  bool   processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon);

  bool legacyContainer;
  bool enableSingleRetracking;

  int charge1, charge2;

  std::string geom_file_name;
  SQGenFit::GFField* gfield;

  SRecEvent*      recEvent;
  SQTrackVector*  recTrackVec;

  SQDimuonVector* recDimuonVec;
};

#endif