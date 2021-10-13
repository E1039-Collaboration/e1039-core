#include <iomanip>
#include <algorithm>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include "SQGeomAcc.h"
using namespace std;

SQGeomAcc::SQGeomAcc(const string& name)
  : SubsysReco   (name)
  , m_mode_muon  (UNDEF_MUON)
  , m_mode_plane (UNDEF_PLANE)
  , m_n_ele_h1_ex(0)
  , m_vec_hit    (0)
{
  ;
}

SQGeomAcc::~SQGeomAcc()
{
  ;
}

int SQGeomAcc::Init(PHCompositeNode* topNode)
{
  if (m_mode_muon == UNDEF_MUON) {
    cout << Name() << ": The muon mode is not selected.  Abort." << endl;
    exit(1);
  }
  if (m_mode_plane == UNDEF_PLANE) {
    cout << Name() << ": The plane mode is not selected.  Abort." << endl;
    exit(1);
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQGeomAcc::InitRun(PHCompositeNode* topNode)
{
  m_vec_hit = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if(!m_vec_hit) {
    cerr << Name() << ":  Failed at getting SQHitVector.  Abort." << endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQGeomAcc::process_event(PHCompositeNode* topNode)
{
  /// Prepare a set of detector planes used to define the geometric acceptance.
  /// Planes are selected by their names for convenience and convert names to IDs for process speed.
  const vector<string> list_hodo_top_name = { "H1T", "H2T", "H3T", "H4T" };
  const vector<string> list_hodo_bot_name = { "H1B", "H2B", "H3B", "H4B" };
  const vector<string> list_cham_top_name = { "D0X", "D2X", "D3pX" };
  const vector<string> list_cham_bot_name = { "D0X", "D2X", "D3mX" };
  static vector<int> list_top_id;
  static vector<int> list_bot_id;
  static int h1t_id = 0;
  static int h1b_id = 0;
  if (list_top_id.size() == 0) { // Initialize
    if (m_mode_plane == HODO || m_mode_plane == HODO_CHAM) { // Add hodoscope planes
      for (unsigned int ii = 0; ii < list_hodo_top_name.size(); ii++) list_top_id.push_back(GetDetId(list_hodo_top_name[ii]));
      for (unsigned int ii = 0; ii < list_hodo_bot_name.size(); ii++) list_bot_id.push_back(GetDetId(list_hodo_bot_name[ii]));
      h1t_id = GetDetId("H1T");
      h1b_id = GetDetId("H1B");
    }
    if (m_mode_plane == CHAM || m_mode_plane == HODO_CHAM) { // Add chamber planes
      for (unsigned int ii = 0; ii < list_cham_top_name.size(); ii++) list_top_id.push_back(GetDetId(list_cham_top_name[ii]));
      for (unsigned int ii = 0; ii < list_cham_bot_name.size(); ii++) list_bot_id.push_back(GetDetId(list_cham_bot_name[ii]));
    }
  }

  /// Extract and arrange the contents of SQHitVector.
  /// What we need here is a list of planes having hits per true track.
  map< int, vector<int> > map_vec_det_id; // [track ID] -> vector<detector ID>
  for (SQHitVector::ConstIter it = m_vec_hit->begin(); it != m_vec_hit->end(); it++) {
    SQHit* hit = *it;
    int det_id = hit->get_detector_id();
    if (m_n_ele_h1_ex > 0 && (det_id == h1t_id || det_id == h1b_id)) { // Consider excluding edge elements
      int ele_id = hit->get_element_id();
      if (ele_id <= m_n_ele_h1_ex || ele_id > GeomSvc::instance()->getPlaneNElements(det_id) - m_n_ele_h1_ex) continue;
    }
    map_vec_det_id[ hit->get_track_id() ].push_back(det_id);
  }

  /// Check if each true track has a hit on all required planes and count up such tracks
  int n_trk_top = 0; // N of tracks that make hits on all top planes
  int n_trk_bot = 0; // N of tracks that make hits on all bottom planes
  for (map< int, vector<int> >::iterator it = map_vec_det_id.begin(); it != map_vec_det_id.end(); it++) {
    if (FindDetIdSet(it->second, list_top_id)) n_trk_top++;
    if (FindDetIdSet(it->second, list_bot_id)) n_trk_bot++;
  }

  return ( (m_mode_muon == SINGLE    &&  n_trk_top + n_trk_bot >= 1)
        || (m_mode_muon == SINGLE_T  &&  n_trk_top             >= 1)
        || (m_mode_muon == SINGLE_B  &&              n_trk_bot >= 1)
        || (m_mode_muon == PAIR      &&  n_trk_top + n_trk_bot >= 2)
        || (m_mode_muon == PAIR_TBBT &&  n_trk_top >= 1 && n_trk_bot >= 1 )
        || (m_mode_muon == PAIR_TTBB && (n_trk_top >= 2 || n_trk_bot >= 2))
    ) ? Fun4AllReturnCodes::EVENT_OK : Fun4AllReturnCodes::ABORTEVENT;
}

int SQGeomAcc::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

/// Return the detector ID for `det_name` after assuring that the detector ID is valid (i.e. non-zero).
int SQGeomAcc::GetDetId(const std::string& det_name)
{
  int det_id = GeomSvc::instance()->getDetectorID(det_name);
  if (det_id <= 0) {
    cout << PHWHERE << ": No detector ID found for '" << det_name << "'.  Abort." << endl;
    exit(1);
  }
  return det_id;
}

/// Return 'true' if `vec_det_id_all` contains _all_ elements of `vec_det_id_want`.
bool SQGeomAcc::FindDetIdSet(const std::vector<int>& vec_det_id_all, const std::vector<int>& vec_det_id_want)
{
  for (unsigned int ii = 0; ii < vec_det_id_want.size(); ii++) {
    int det_id = vec_det_id_want[ii];
    if (find(vec_det_id_all.begin(), vec_det_id_all.end(), det_id) == vec_det_id_all.end()) return false;
  }
  return true;
}
