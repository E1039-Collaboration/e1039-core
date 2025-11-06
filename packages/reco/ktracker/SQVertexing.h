#ifndef _SQVERTEXING_H
#define _SQVERTEXING_H

#include <TRandom2.h>
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

/// SubsysReco module to carry out the vertexing (i.e. forming dimuons).
/**
 * This class can be used outside the Fun4All macro, 
 * where some member functions are called with topNode = 0 in such use.
 */
class SQVertexing: public SubsysReco
{
public:
  SQVertexing(const std::string& name = "SQVertexing", int sign1 = 1, int sign2 = -1);
  ~SQVertexing();

  int Init(PHCompositeNode* topNode=0);
  int InitRun(PHCompositeNode* topNode=0);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void set_legacy_rec_container(const bool enable = true)  { legacyContainer_in  = enable; legacyContainer_out = enable; }
  void set_legacy_in_container(const bool enable = true)   { legacyContainer_in  = enable; }
  void set_legacy_out_container(const bool enable = true)  { legacyContainer_out = enable; }
  void set_single_retracking(const bool enable = true)     { enableSingleRetracking = true; }

  void set_geom_file_name(const std::string& geomFileName) { geom_file_name = geomFileName; }

private:
  int InitField(PHCompositeNode* topNode=0);
  int InitGeom(PHCompositeNode*  topNode=0);
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode*  topNode);

  double swimTrackToVertex(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double refitTrkToVtx(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double refitTrkToVtx(SRecTrack*         track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2);
  double calcZsclp(double p);
  bool   processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon);
  bool   processOneMuon(SRecTrack* track);

  bool legacyContainer_in, legacyContainer_out;
  bool enableSingleRetracking;

  int charge1, charge2;

  TRandom2 rndm;

  std::string geom_file_name;
  SQGenFit::GFField* gfield;

  SRecEvent*      recEvent;
  SQTrackVector*  recTrackVec;

  SQDimuonVector* recDimuonVec;
};

#endif
