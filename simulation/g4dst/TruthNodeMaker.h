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
/**
 * This module adds the following SQ nodes to the DST top node;
 *  - SQMCEvent = info on the primary process like Drell-Yan process (cf. class `SQMCEvent`)
 *  - SQTruthTrackVector = list of true tracks (cf. class `SQTrackVector`)
 *  - SQTruthDimuonVector = list of true dimuons (cf. class `SQDimuonVector`)
 *
 * The contents of these nodes are not original but extracted from `HepMC::GenEvent`, `PHG4TruthInfoContainer`, etc.
 * Thus you can extract the same or more detailed info when necessary.
 *
 * The info on the true tracks is stored in `HepMC::GenEvent` as well,
 * where it is extracted from `PHG4TruthInfoContainer` in this module.
 * It is because the positions/momenta of tracks at Stations 1 and 3 are accessible only via `PHG4TruthInfoContainer`.
 * As a side effect, the parent particle type (i.e. PDG ID) of each muon pair is not available now, which is stored only in `HepMC::GenEvent`.
 * Analyzer should use the (true) invariant mass of muon pair to identify its parent particle type.
 */
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
