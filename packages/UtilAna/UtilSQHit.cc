#include <iomanip>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include "UtilSQHit.h"
using namespace std;

/**
 * The SQHitVector object returned has to be deleted outside this function.
 * The recommended way is to use "shared_ptr", which auto-deletes the object.
 * @code
 *   shared_ptr<SQHitVector> hv_h1t(UtilSQHit::FindHits(hit_vec, "H1T"));
 *   cout << "N = " << hv_h1t->size() << endl;
 * @endcode
 */
SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const std::string det_name, const bool in_time)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindHits(vec_in, geom->getDetectorID(det_name), in_time);
}

/**
 * See the other FindHits() function to find how to handle the returned object.
 */
SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const int det_id, const bool in_time)
{
  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (SQHitVector::ConstIter it = vec_in->begin(); it != vec_in->end(); it++) {
    SQHit* hit = *it;
    if (in_time && ! hit->is_in_time()) continue;
    if (hit->get_detector_id() == det_id) vec->push_back(hit);
  }
  return vec;
}

/**
 * See FindHits() to find how to handle the returned object.
 */
SQHitVector* UtilSQHit::FindFirstHits(const SQHitVector* vec_in, const std::string det_name, const bool in_time)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindFirstHits(vec_in, geom->getDetectorID(det_name), in_time);
}

/**
 * See FindHits() to find how to handle the returned object.
 */
SQHitVector* UtilSQHit::FindFirstHits(const SQHitVector* vec_in, const int det_id, const bool in_time)
{
  map<int, double> id2time; // [element ID] = first (max) tdcTime;
  map<int, int   > id2idx ; // [element ID] = index of vec_in
  for (unsigned int idx = 0; idx < vec_in->size(); idx++) {
    const SQHit* hit = vec_in->at(idx);
    if (in_time && ! hit->is_in_time()) continue;
    if (hit->get_detector_id() != det_id) continue;
    short  ele_id = hit->get_element_id();
    double time   = hit->get_tdc_time();
    if (id2time.find(ele_id) == id2time.end() || time > id2time[ele_id]) {
      id2time[ele_id] = time;
      id2idx [ele_id] = idx;
    }
  }

  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (map<int, int>::iterator it = id2idx.begin(); it != id2idx.end(); it++) {
    vec->push_back(vec_in->at(it->second));
  }
  return vec;
}

/**
 * See `FindHitsFast(evt, hit_vec, det_id)` for details.
 */
std::vector<SQHit*>* UtilSQHit::FindHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const std::string det_name)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindHitsFast(evt, hit_vec, geom->getDetectorID(det_name));
}

/// Fast-extract a set of hits that are of the given detector (det_id).
/**
 * Useful for speed when many SubsysReco modules are registered and accessing the hit vector repeatedly.
 * The returned variable is `vector<SQHit*>` (not `SQHitVector`), which can be used as follows for example:
 * @code
 *   auto vec = UtilSQHit::FindHitsFast(evt, hit_vec, det_id);
 *   for (auto it = vec->begin(); it != vec->end(); it++) {
 *     int    ele_id = (*it)->get_element_id();
 *     double time   = (*it)->get_tdc_time  ();
 *   }
 * @endcode
 * Caller must _not_ delete the vector nor the SQHit objects.
 *
 * This function has an internal static variable that holds one hit vector per detector.
 * The variable is filled only on the 1st call per event, and used by all SubsysReco modules.
 * The reason of using `vector<SQHit*>` is to avoid multiple creations+deletions of `SQHitVector`.
 */
std::vector<SQHit*>* UtilSQHit::FindHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const int det_id)
{
  static int run_id_now = 0;
  static int evt_id_now = 0;
  static map< int, vector<SQHit*> > hit_vec_map; // [det_id] => hit_vec

  int run_id = evt->get_run_id();
  int evt_id = evt->get_event_id();
  if (run_id_now == 0 || run_id != run_id_now || evt_id != evt_id_now) {
    hit_vec_map.clear();
    for (auto it = hit_vec->begin(); it != hit_vec->end(); it++) {
      SQHit* hit = *it;
      hit_vec_map[hit->get_detector_id()].push_back(hit);
    }
    run_id_now = run_id;
    evt_id_now = evt_id;
  }

  return &hit_vec_map[det_id];
}

/**
 * See `FindTriggerHitsFast(evt, hit_vec, det_id)` for details.
 */
std::vector<SQHit*>* UtilSQHit::FindTriggerHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const std::string det_name)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindTriggerHitsFast(evt, hit_vec, geom->getDetectorID(det_name));
}

/// Fast-extract a set of trigger hits that are of the given detector (det_id).
/**
 * See `FindHitsFast()` for usage.
 */
std::vector<SQHit*>* UtilSQHit::FindTriggerHitsFast(const SQEvent* evt, const SQHitVector* hit_vec, const int det_id)
{
  static int run_id_now = 0;
  static int evt_id_now = 0;
  static map< int, vector<SQHit*> > hit_vec_map; // [det_id] => hit_vec

  int run_id = evt->get_run_id();
  int evt_id = evt->get_event_id();
  if (run_id_now == 0 || run_id != run_id_now || evt_id != evt_id_now) {
    hit_vec_map.clear();
    for (auto it = hit_vec->begin(); it != hit_vec->end(); it++) {
      SQHit* hit = *it;
      hit_vec_map[hit->get_detector_id()].push_back(hit);
    }
    run_id_now = run_id;
    evt_id_now = evt_id;
  }

  return &hit_vec_map[det_id];
}
