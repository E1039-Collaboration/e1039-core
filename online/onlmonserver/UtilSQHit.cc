#include <iomanip>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQHitVector.h>
#include "UtilSQHit.h"
using namespace std;

SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const std::string det_name)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindHits(vec_in, geom->getDetectorID(det_name));
}

SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const int det_id)
{
  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (SQHitVector::ConstIter it = vec_in->begin(); it != vec_in->end(); it++) {
    SQHit* hit = *it;
    if (hit->get_detector_id() == det_id) vec->push_back(hit);
  }
  return vec;
}

SQHitVector* UtilSQHit::FindFirstHits(const SQHitVector* vec_in, const std::string det_name)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindFirstHits(vec_in, geom->getDetectorID(det_name));
}

SQHitVector* UtilSQHit::FindFirstHits(const SQHitVector* vec_in, const int det_id)
{
  map<int, double> id2time; // [element ID] = first (max) tdcTime;
  map<int, int   > id2idx ; // [element ID] = index of vec_in
  for (unsigned int idx = 0; idx < vec_in->size(); idx++) {
    const SQHit* hit = vec_in->at(idx);
    if (hit->get_detector_id() != det_id) continue;
    short  ele_id = hit->get_element_id();
    double time   = hit->get_tdc_time();
    if (id2time.find(ele_id) != id2time.end() || time > id2time[ele_id]) {
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
