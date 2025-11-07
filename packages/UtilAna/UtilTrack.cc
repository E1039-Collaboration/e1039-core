#include <cassert>
#include <geom_svc/GeomSvc.h>
#include <ktracker/SRecEvent.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQHitVector.h>
#include "UtilTrigger.h"
#include "UtilTrack.h"
using namespace std;

int UtilTrack::verbosity = 0;

/// Find a track by track ID in the given track list.
/**
 * This function returns a SQTrack pointer if successful.
 * Otherwise it returns '0' by default, or aborts when 'do_assert' = true.
 */
SQTrack* UtilTrack::FindTrackByID(const SQTrackVector* vec, const int id_trk, const bool do_assert)
{
  for (SQTrackVector::ConstIter it = vec->begin(); it != vec->end(); it++) {
    SQTrack* trk = *it;
    if (trk->get_track_id() == id_trk) {
      if (do_assert) assert(trk);
      return trk;
    }
  }
  return 0;
}

/// Find all hits associated with the given track.
/**
 * It is recommended to receive the returned pointer by "shared_ptr".
 * @code
 *   shared_ptr<SQHitVector> hit_vec_trk(UtilTrack::FindHitsOfTrack(hit_vec, id_trk));
 * @endcode
 */
SQHitVector* UtilTrack::FindHitsOfTrack(const SQHitVector* vec_in, const int id_trk)
{
  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (SQHitVector::ConstIter it = vec_in->begin(); it != vec_in->end(); it++) {
    SQHit* hit = *it;
    if (hit->get_track_id() == id_trk) vec->push_back(hit);
  }
  return vec;
}

/// Find track-associated hits whose detector name starts with 'det_name'.
/**
 * Examples:
 * @code
 *   shared_ptr<SQHitVector> hit_vec_trk_d0(UtilTrack::FindDetectorHitsOfTrack(hit_vec, id_trk, "D0"));
 * @endcode
 *
 * You can use "FindHodoHitsOfTrack()" to more-conveniently find all hodoscope hits.
 */
SQHitVector* UtilTrack::FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const std::string det_name)
{
  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (SQHitVector::ConstIter it = vec_in->begin(); it != vec_in->end(); it++) {
    SQHit* hit = *it;
    if (hit->get_track_id() != id_trk) continue;
    string name = GeomSvc::instance()->getDetectorName( hit->get_detector_id() );
    if (name.substr(0, det_name.size()) != det_name) continue;
    vec->push_back(hit);
  }
  return vec;
}

/// Find track-associated hits whose detector name starts with 'det_name'.
SQHitVector* UtilTrack::FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const char* det_name)
{
  return FindDetectorHitsOfTrack(vec_in, id_trk, (const string)det_name);
}

/// Find all hodoscope hits hits associated with the given track.
/**
 * Example:
 * @code
 *   shared_ptr<SQHitVector> hit_vec_trk(UtilTrack::FindHodoHitsOfTrack(hit_vec, id_trk));
 * @endcode
 */
SQHitVector* UtilTrack::FindHodoHitsOfTrack(const SQHitVector* vec_in, const int id_trk)
{
  return FindDetectorHitsOfTrack(vec_in, id_trk, "H");
}

/// Find all roads that match to the given track within the hodo paddle width plus the given margin.
/**
 * The closest paddle to the given track at each hodoscope X plane (H1T/B, H2T/B, H3T/B and H4T/B) is selected based on the expected track x-position.
 * The two adjacent paddles are also selected if they are within "width/2 + margin".
 * All selected paddles are used to list up matched roads.
 * @code
 *   SRecTrack* trk = ...
 *   const double margin = 1.0; // in cm
 *   vector<int> list_road_id = UtilTrack::FindMatchedRoad(trk, margin);
 * @endcode
 */
std::vector<int> UtilTrack::FindMatchedRoads(SRecTrack* trk, const double margin)
{
  return FindMatchedRoads(trk->get_pos_st1(), trk->get_mom_st1(), trk->get_pos_st3(), trk->get_mom_st3(), margin);
}

/// Find all roads that match to the given track within the hodo paddle width plus the given margin.
std::vector<int> UtilTrack::FindMatchedRoads(const TVector3 pos1, const TLorentzVector mom1, const TVector3 pos3, const TLorentzVector mom3, const double margin)  
{
  GeomSvc* geom = GeomSvc::instance();
  double y_st1 = pos1.Y();
  double y_st3 = pos3.Y();
  int top_bot = y_st3>0 ? +1 : -1;
  if (verbosity > 0) cout << "UtilTrack::FindMatchedRoads(): y_st1=" << y_st1 << " y_st3=" << y_st3 << " top_bot=" << top_bot << endl;
  
  vector<int> list_ele_id[5]; // 0=unused, 1=st1, 2=st2,,,
  for (int st = 1; st <= 4; st++) {
    string det_name = (string)"H" + (char)('0'+st) + (top_bot>0 ? 'T' : 'B');
    int det_id = geom->getDetectorID(det_name);
    Plane* det = geom->getPlanePtr(det_id);
    double x_det = det->xc + det->deltaX;
    //double y_det = det->yc + det->deltaY;
    double z_det = det->zc + det->deltaZ;
    int    n_ele = det->nElements;
    double space = det->spacing;
    double width = det->cellWidth;
    if (verbosity > 2) cout << "  st" << st << ":";
    if (verbosity > 3) cout << " x_det=" << x_det << " n_ele=" << n_ele << " space=" << space << " width=" << width;

    const TVector3*       pos = (st == 1 ? &pos1 : &pos3);
    const TLorentzVector* mom = (st == 1 ? &mom1 : &mom3);
    double x_trk = pos->X() + (z_det - pos->Z()) * mom->X() / mom->Z();
    double y_trk = pos->Y() + (z_det - pos->Z()) * mom->Y() / mom->Z();
    //double x_trk, y_trk;
    //trk->getExpPositionFast(z_det, x_trk, y_trk);
    int ele_cent = (int)((n_ele+1)/2.0 + (x_trk-x_det)/space + 0.5);
    if (verbosity > 2) cout << " x_trk=" << x_trk << " y_trk=" << y_trk;

    /// Check three elements, assuming the margin is much smaller than the half width.
    for (int i_ele = ele_cent - 1; i_ele <= ele_cent + 1; i_ele++) {
      if (i_ele <= 0 || i_ele > n_ele) continue;
      double x_ele = x_det + space * (i_ele - (n_ele+1)/2.0);
      if (verbosity > 2) cout << " [ele=" << i_ele << " x=" << x_ele;
      if (verbosity > 3) cout << " edge=" << (width/2-fabs(x_trk-x_ele));
      if (verbosity > 2) cout << "]";
      if (fabs(x_trk - x_ele) < width/2 + margin) list_ele_id[st].push_back(i_ele);
    }
    if (verbosity > 2) cout << endl;
  }
  vector<int> list_road_id;
  for (auto it1 = list_ele_id[1].begin(); it1 != list_ele_id[1].end(); it1++) {
  for (auto it2 = list_ele_id[2].begin(); it2 != list_ele_id[2].end(); it2++) {
  for (auto it3 = list_ele_id[3].begin(); it3 != list_ele_id[3].end(); it3++) {
  for (auto it4 = list_ele_id[4].begin(); it4 != list_ele_id[4].end(); it4++) {
    if (verbosity > 0) cout << "  road " << *it1 << " " << *it2 << " " << *it3 << " " << *it4;
    if (verbosity > 1) cout << " " << UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, top_bot);
    if (verbosity > 0) cout << endl;
    list_road_id.push_back(UtilTrigger::Hodo2Road(*it1, *it2, *it3, *it4, top_bot));
  }}}}
  return list_road_id;
}
