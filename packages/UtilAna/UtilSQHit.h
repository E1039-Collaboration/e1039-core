#ifndef _UTIL_SQHIT__H_
#define _UTIL_SQHIT__H_
class SQHitVector;

/// A set of utility functions about SQHit.
namespace UtilSQHit {
  /// Extract a set of hits that are of the given detector (det_name).
  SQHitVector* FindHits(const SQHitVector* vec_in, const std::string det_name);
  SQHitVector* FindHits(const SQHitVector* vec_in, const int         det_id  );

  /// Extract a set of first hits that are of the given detector (det_name), where "first" means the earliest (i.e. largest TDC time) hit per element.
  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const std::string det_name);
  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const int         det_id  );
};

#endif /* _UTIL_SQHIT__H_ */
