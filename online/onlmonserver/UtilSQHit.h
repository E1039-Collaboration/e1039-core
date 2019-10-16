#ifndef _UTIL_SQHIT__H_
#define _UTIL_SQHIT__H_
class SQHitVector;

namespace UtilSQHit {
  SQHitVector* FindHits(const SQHitVector* vec_in, const std::string det_name);
  SQHitVector* FindHits(const SQHitVector* vec_in, const int         det_id  );

  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const std::string det_name);
  SQHitVector* FindFirstHits(const SQHitVector* vec_in, const int         det_id  );
};

#endif /* _UTIL_SQHIT__H_ */
