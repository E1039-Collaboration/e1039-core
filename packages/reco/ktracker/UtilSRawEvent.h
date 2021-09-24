#ifndef _UTIL_SRAWEVENT__H_
#define _UTIL_SRAWEVENT__H_
#include <map>
class SRawEvent;
class SQEvent;
class SQSpillMap;
class SQSpill;
class SQHitVector;

/// A set of utility functions about SRawEvent.
namespace UtilSRawEvent {
  bool SetEvent     (SRawEvent* sraw, const SQEvent*     evt    , const bool do_assert=false);
  bool SetSpill     (SRawEvent* sraw, const SQSpill*     sp     , const bool do_assert=false);
  bool SetHit       (SRawEvent* sraw, const SQHitVector* hit_vec, std::map<int, size_t>* hitID_idx=0, const bool do_assert=false);
  bool SetTriggerHit(SRawEvent* sraw, const SQHitVector* hit_vec, std::map<int, size_t>* hitID_idx=0, const bool do_assert=false);
};

#endif // _UTIL_SRAWEVENT__H_
