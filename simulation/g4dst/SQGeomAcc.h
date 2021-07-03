#ifndef _SQ_GEOM_ACC__H_
#define _SQ_GEOM_ACC__H_
#include <map>
#include <fun4all/SubsysReco.h>
class SQHitVector;

/// An SubsysReco module to skip a simulated event in which a muon or a muon pair doesn't pass through the geometric acceptance.
/**
 * Typical usage:
 * ```
 *   SQGeomAcc* geom_acc = new SQGeomAcc();
 *   geom_acc->SetMuonMode(SQGeomAcc::PAIR_TBBT);
 *   geom_acc->SetPlaneMode(SQGeomAcc::HODO_CHAM);
 *   se->registerSubsystem(geom_acc);
 * ```
 *
 * All the available modes are listed and explained in the sections of `MuonMode_t` and `PlaneMode_t`.
 * This module must be registered after `SQDigitizer` since it uses SQHits.
 */
class SQGeomAcc: public SubsysReco {
 public:
  typedef enum {
    UNDEF_MUON, 
    SINGLE,    //< Require one muon at either top or bottom half
    SINGLE_T,  //< Require one muon at top half
    SINGLE_B,  //< Require one muon at bottom half
    PAIR,      //< Require two muons at any halves
    PAIR_TBBT, //< Require two muons at T+B or B+T halves
    PAIR_TTBB, //< Require two muons at T+T or B+B halves
  } MuonMode_t;
  typedef enum {
    UNDEF_PLANE, 
    HODO,      //< Use the hodoscope acceptance
    CHAM,      //< Use the chamber acceptance
    HODO_CHAM, //< Use the hodoscope+chamber acceptance
  } PlaneMode_t;

 private:
  MuonMode_t   m_mode_muon;
  PlaneMode_t  m_mode_plane;
  SQHitVector* m_vec_hit;

 public:
  SQGeomAcc(const std::string& name = "SQGeomAcc");
  virtual ~SQGeomAcc();
  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  void SetMuonMode (MuonMode_t  mode) { m_mode_muon  = mode; }
  void SetPlaneMode(PlaneMode_t mode) { m_mode_plane = mode; }

 private:
  int  GetDetId(const std::string& det_name);
  bool FindDetIdSet(const std::vector<int>& vec_det_id_all, const std::vector<int>& vec_det_id_want);
};

#endif /* _SQ_GEOM_ACC__H_ */
