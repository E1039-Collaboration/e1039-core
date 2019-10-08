#ifndef _UTIL_SQHIT__H_
#define _UTIL_SQHIT__H_
class SQHitVector;

namespace UtilSQHit {
  SQHitVector* FindHits(const SQHitVector* vec_in, const std::string plane);
  SQHitVector* FindHits(const SQHitVector* vec_in, const int         plane);
};

#endif /* _UTIL_SQHIT__H_ */
