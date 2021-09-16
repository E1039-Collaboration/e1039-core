//#include <iomanip>
#include <phool/phool.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQHitVector.h>
#include <ktracker/SRawEvent.h>
#include "UtilSRawEvent.h"
using namespace std;

void UtilSRawEvent::SetEvent(SRawEvent* sraw, const SQEvent* evt, const bool do_assert)
{
  if (!evt) {
    if (do_assert) {
      cout << PHWHERE << ": SQEvent == 0.  Abort." << endl;
      exit(1);
    }
    return;
  }
  int run_id   = evt->get_run_id();
  int spill_id = evt->get_spill_id();
  int event_id = evt->get_event_id();
  sraw->setEventInfo(run_id, spill_id, event_id);

  int triggers[10];
  for(int i = SQEvent::NIM1; i <= SQEvent::MATRIX5; ++i) {
    triggers[i] = evt->get_trigger(static_cast<SQEvent::TriggerMask>(i));
  }
  sraw->setTriggerBits(triggers);
}

void UtilSRawEvent::SetSpill(SRawEvent* sraw, const SQSpill* sp, const bool do_assert)
{
  if (!sp) {
    if (do_assert) {
      cout << PHWHERE << ": SQSpill == 0.  Abort." << endl;
      exit(1);
    }
    return;
  }
  sraw->setTargetPos(sp->get_target_pos());
}

void UtilSRawEvent::SetHit(SRawEvent* sraw, const SQHitVector* hit_vec, const bool do_assert)
{
  sraw->emptyHits();
  if (!hit_vec) {
    if (do_assert) {
      cout << PHWHERE << ": SQHitVector == 0.  Abort." << endl;
      exit(1);
    }
    return;
  }
  for(size_t idx = 0; idx < hit_vec->size(); ++idx) {
    const SQHit* sq_hit = hit_vec->at(idx);
    Hit h;
    h.index         = sq_hit->get_hit_id();
    h.detectorID    = sq_hit->get_detector_id();
    h.elementID     = sq_hit->get_element_id();
    h.tdcTime       = sq_hit->get_tdc_time();
    h.driftDistance = fabs(sq_hit->get_drift_distance()); //MC L-R info removed here
    h.pos           = sq_hit->get_pos();
    if(sq_hit->is_in_time()) h.setInTime();
    sraw->insertHit(h);
  }
  sraw->reIndex(true);
}

void UtilSRawEvent::SetTriggerHit(SRawEvent* sraw, const SQHitVector* hit_vec, const bool do_assert)
{
  sraw->emptyTriggerHits();
  if (!hit_vec) {
    if (do_assert) {
      cout << PHWHERE << ": SQHitVector == 0.  Abort." << endl;
      exit(1);
    }
    return;
  }
  for(size_t idx = 0; idx < hit_vec->size(); ++idx) {
    const SQHit* sq_hit = hit_vec->at(idx);
    Hit h;
    h.index         = sq_hit->get_hit_id();
    h.detectorID    = sq_hit->get_detector_id();
    h.elementID     = sq_hit->get_element_id();
    h.tdcTime       = sq_hit->get_tdc_time();
    h.driftDistance = fabs(sq_hit->get_drift_distance()); //MC L-R info removed here
    h.pos           = sq_hit->get_pos();
    if(sq_hit->is_in_time()) h.setInTime();
    sraw->insertTriggerHit(h);
  }
}
