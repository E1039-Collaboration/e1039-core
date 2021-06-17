#ifndef _UTIL_TRACK__H_
#define _UTIL_TRACK__H_
class SQTrack;
class SQTrackVector;
class SQHitVector;

namespace UtilTrack {
  SQTrack* FindTrackByID(const SQTrackVector* vec, const int id_trk, const bool do_assert=false);

  SQHitVector* FindHitsOfTrack(const SQHitVector* vec_in, const int id_trk);
  SQHitVector* FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const std::string det_name);
  SQHitVector* FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const char*       det_name);
  SQHitVector* FindHodoHitsOfTrack(const SQHitVector* vec_in, const int id_trk);
}

#endif /* _UTIL_TRACK__H_ */
