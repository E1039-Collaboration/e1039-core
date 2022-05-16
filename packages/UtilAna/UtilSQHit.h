#ifndef _UTIL_SQHIT__H_
#define _UTIL_SQHIT__H_
class SQEvent;
class SQHitVector;

/// A set of utility functions about SQHit.
namespace UtilSQHit {
  /// Extract a set of hits that are of the given detector (det_name).
  SQHitVector* FindHits(const SQHitVector* vec_in, const std::string det_name, const bool in_time=false);
  SQHitVector* FindHits(const SQHitVector* vec_in, const int         det_id  , const bool in_time=false);

  /// Extract a set of first hits that are of the given detector (det_name), where "first" means the earliest (i.e. largest TDC time) hit per element.
  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const std::string det_name, const bool in_time=false);
  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const int         det_id  , const bool in_time=false);

  /// Fast-extract a set of hits that are of the given detector (det_name).
  std::vector<SQHit*>* FindHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const std::string det_name);
  std::vector<SQHit*>* FindHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const int         det_id);

  /// Fast-extract a set of trigger hits that are of the given detector (det_name).
  std::vector<SQHit*>* FindTriggerHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const std::string det_name);
  std::vector<SQHit*>* FindTriggerHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const int         det_id);
};

#endif /* _UTIL_SQHIT__H_ */
