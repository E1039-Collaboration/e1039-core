#include <iomanip>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQHitVector.h>
#include "UtilSQHit.h"
using namespace std;

SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const std::string plane)
{
  GeomSvc* geom = GeomSvc::instance();
  return FindHits(vec_in, geom->getDetectorID(plane));
}

SQHitVector* UtilSQHit::FindHits(const SQHitVector* vec_in, const int plane)
{
  SQHitVector* vec = vec_in->Clone();
  vec->clear();
  for (SQHitVector::ConstIter it = vec_in->begin(); it != vec_in->end(); it++) {
    SQHit* hit = *it;
    if (hit->get_detector_id() == plane) vec->push_back(hit);
  }
  return vec;
}
