#ifndef _UTIL_SRAWEVENT__H_
#define _UTIL_SRAWEVENT__H_
class SRawEvent;
class SQEvent;
class SQSpillMap;
class SQSpill;
class SQHitVector;

/// A set of utility functions about SRawEvent.
namespace UtilSRawEvent {
  void SetEvent     (SRawEvent* sraw, const SQEvent*     evt    , const bool do_assert=false);
  void SetSpill     (SRawEvent* sraw, const SQSpill*     sp     , const bool do_assert=false);
  void SetHit       (SRawEvent* sraw, const SQHitVector* hit_vec, const bool do_assert=false);
  void SetTriggerHit(SRawEvent* sraw, const SQHitVector* hit_vec, const bool do_assert=false);
};

#endif // _UTIL_SRAWEVENT__H_
