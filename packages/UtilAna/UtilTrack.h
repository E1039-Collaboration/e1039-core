#ifndef _UTIL_TRACK__H_
#define _UTIL_TRACK__H_
#include <vector>
class SQTrack;
class SQTrackVector;
class SQHitVector;
class SRecTrack;
class TVector3;
class TLorentzVector;

namespace UtilTrack {
  extern int verbosity;
  SQTrack* FindTrackByID(const SQTrackVector* vec, const int id_trk, const bool do_assert=false);

  SQHitVector* FindHitsOfTrack(const SQHitVector* vec_in, const int id_trk);
  SQHitVector* FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const std::string det_name);
  SQHitVector* FindDetectorHitsOfTrack(const SQHitVector* vec_in, const int id_trk, const char*       det_name);
  SQHitVector* FindHodoHitsOfTrack(const SQHitVector* vec_in, const int id_trk);

  std::vector<int> FindMatchedRoads(SRecTrack* trk, const double margin=0);
  std::vector<int> FindMatchedRoads(const TVector3 pos1, const TLorentzVector mom1, const TVector3 pos3, const TLorentzVector mom3, const double margin=0);
}

#endif /* _UTIL_TRACK__H_ */
