#ifndef _TRUTH_NODE_MAKER__H_
#define _TRUTH_NODE_MAKER__H_
#include <fun4all/SubsysReco.h>
class TVector3;
class TLorentzVector;
namespace HepMC { 
  class GenParticle; 
};
class PHHepMCGenEventMap;
class PHG4TruthInfoContainer;
class PHG4HitContainer;
class SQMCEvent;
class SQTrack;
class SQTrackVector;
class SQDimuonVector;

/// An SubsysReco module to create a set of SQ nodes for the simulation true info.
class TruthNodeMaker: public SubsysReco {
  PHHepMCGenEventMap* genevtmap;
  PHG4TruthInfoContainer* g4true;
  PHG4HitContainer *g4hc_d1x;
  PHG4HitContainer *g4hc_d3px;
  PHG4HitContainer *g4hc_d3mx;

  SQMCEvent*      m_evt;
  SQTrackVector*  m_vec_trk;
  SQDimuonVector* m_vec_dim;

 public:
  TruthNodeMaker();
  virtual ~TruthNodeMaker();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

 private:
  int  GetNodes(PHCompositeNode *topNode);
  int MakeNodes(PHCompositeNode *topNode);
  bool FindHitAtStation(const int trk_id, const PHG4HitContainer* g4hc, TVector3* pos, TLorentzVector* mom);
};

#endif /* _TRUTH_NODE_MAKER__H_ */
