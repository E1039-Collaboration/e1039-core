#ifndef _SQVERTEXING_H
#define _SQVERTEXING_H

#include <fun4all/SubsysReco.h>

#include "GFField.h"
#include "GFTrack.h"

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

  void set_legacy_rec_container(const bool enable = true) { legacyContainer = enable; }
  void setFieldPtr(SQGenFit::GFField* field) { gfield = field; }

private:
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode* topNode);

  double swimTrackToVertex(SQGenFit::GFTrack& track, double z, TVector3* pos = nullptr, TVector3* mom = nullptr);
  double findDimuonZVertex(SRecDimuon& dimuon, SQGenFit::GFTrack& track1, SQGenFit::GFTrack& track2);
  bool   processOneDimuon(SRecTrack* track1, SRecTrack* track2, SRecDimuon& dimuon);

  bool legacyContainer;

  int charge1, charge2;

  SQGenFit::GFField* gfield;

  SRecEvent*      recEvent;
  SQTrackVector*  recTrackVec;

  SQDimuonVector* recDimuonVec;
};

#endif