#include <cmath>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQHitVector.h>
using namespace std;
namespace UtilTrigger {

/// Convert a set of hodo IDs to a roadset ID.
/**
 * Note that the max hodo IDs are 23 (H1), 16 (H2), 16 (H3) and 16 (H4).
 */
int Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb)
{
  if (h1 * h2 * h3 * h4 * tb == 0) return 0; // if any is 0.
  int road = (h1-1)*16*16*16 + (h2-1)*16*16 + (h3-1)*16 + h4;
  return (tb > 0 ? +1 : -1) * road;
}

/// Convert a roadset ID to a set of hodo IDs.
void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb)
{
  if (road == 0) {
    h1 = h2 = h3 = h4 = tb = 0;
    return;
  }
  int rr = abs(road) - 1;
  h1 = 1 + (rr/16/16/16);
  h2 = 1 + (rr/16/16   ) %16;
  h3 = 1 + (rr/16      ) %16;
  h4 = 1 + (rr         ) %16;
  tb = road>0 ? +1 : -1;
}

/// Find a unique road in the given list of SQHits.
/**
 * This function returns a road ID if the given list contains exactly 
 * one hit every top plane (i.e. H1T, H2T, H3T & H4T) or every bottom plane.
 * It returns '0' if any plan has no hit or multiple hit.
 * The given list can contain any extra Y-hodoscope, chamber or prop tube hit, which is just ignored.
 */
int ExtractRoadID(const SQHitVector* vec)
{
  int tb = 0; // +1 for top, -1 for bottom
  int list_ele_id[5]; // index = station
  memset(list_ele_id, 0, sizeof(list_ele_id));
  for (SQHitVector::ConstIter it = vec->begin(); it != vec->end(); it++) {
    SQHit* hit = *it;
    string name = GeomSvc::instance()->getDetectorName( hit->get_detector_id() );
    if (name[0] != 'H') continue; // Skip non-hodoscope plane
    if        (name[2] == 'T') {
      if (tb < 0) return 0; // Bad as top-bottom mixed up
      tb = +1;
    } else if (name[2] == 'B') {
      if (tb > 0) return 0; // Bad as top-bottom mixed up
      tb = -1;
    } else {
      continue; // Skip non-X plane
    }
    int station = (name[1] - '0');
    if (list_ele_id[station] != 0) return 0; // Bad as multiple hits per station
    list_ele_id[station] = hit->get_element_id();
  }
  return Hodo2Road(list_ele_id[1], list_ele_id[2], list_ele_id[3], list_ele_id[4], tb);
}

}; // End of "namespace UtilTrigger"
