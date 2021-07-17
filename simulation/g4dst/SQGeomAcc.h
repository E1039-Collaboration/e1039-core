#ifndef _SQ_GEOM_ACC__H_
#define _SQ_GEOM_ACC__H_
#include <map>
#include <fun4all/SubsysReco.h>
class SQHitVector;

/// An SubsysReco module to skip a simulated event in which a muon or a muon pair doesn't pass through the geometric acceptance.
/**
 * Typical usage:
 * @code
 *   SQGeomAcc* geom_acc = new SQGeomAcc();
 *   geom_acc->SetMuonMode(SQGeomAcc::PAIR_TBBT);
 *   geom_acc->SetPlaneMode(SQGeomAcc::HODO_CHAM);
 *   geom_acc->SetNumOfH1EdgeElementsExcluded(4); // 0 by default
 *   se->registerSubsystem(geom_acc);
 * @endcode
 *
 * This module must be registered after `SQDigitizer` since it uses SQHits.
 * All the available modes are listed and explained in the sections of `MuonMode_t` and `PlaneMode_t`.
 * Elements at the left and right edges of H1T and H1B can be excluded via `SetNumOfH1EdgeElementsExcluded()`,
 * where the four elements at each edge are planned to be excluded from the trigger logic.
 *
 * The chamber acceptance is defined by D0X, D2X, D3pX and D3mX (not Xp, U, Up, V nor Vp).
 * This definition should be simple and sufficient in terms of the "geometric" acceptance. 
 * D0U/V and D2U/V are identical to D0X and D2X in the plane size.
 * U/V of D3p/m are larger than X but the X-U-V overlapping region is almost identical to X.
 *
 * The effect of detection/reconstruction inefficiencies is not considered by this module.
 * It can/should be studied with other modules, like `SQChamberRealization` and `SQReco`.
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
  unsigned int m_n_ele_h1_ex; //< N of H1T/B elements excluded
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
  void SetNumOfH1EdgeElementsExcluded(const unsigned int num) { m_n_ele_h1_ex = num; }

 private:
  int  GetDetId(const std::string& det_name);
  bool FindDetIdSet(const std::vector<int>& vec_det_id_all, const std::vector<int>& vec_det_id_want);
};

#endif /* _SQ_GEOM_ACC__H_ */
