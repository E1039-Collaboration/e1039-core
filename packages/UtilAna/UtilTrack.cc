#include <cassert>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQTrackVector.h>
#include <interface_main/SQHitVector.h>
#include "UtilTrack.h"
using namespace std;

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
