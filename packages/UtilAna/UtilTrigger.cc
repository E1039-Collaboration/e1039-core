#include <cmath>
#include <geom_svc/GeomSvc.h>
#include <interface_main/SQHitVector.h>
#include "UtilSQHit.h"
#include "TrigRoadset.h"
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

/// Flip the given road ID in the left-right direction.
int FlipRoadLeftRight(const int road)
{
  int h1, h2, h3, h4, tb;
  Road2Hodo(road, h1, h2, h3, h4, tb);
  h1 = 24 - h1; // 1 <-> 23, 2 <-> 22,,,
  h2 = 17 - h2; // 1 <-> 16, 2 <-> 15,,,
  h3 = 17 - h3;
  h4 = 17 - h4;
  return Hodo2Road(h1, h2, h3, h4, tb);
}

/// Flip the given road ID in the top-bottom direction.
int FlipRoadTopBottom(const int road)
{
  int h1, h2, h3, h4, tb;
  Road2Hodo(road, h1, h2, h3, h4, tb);
  return Hodo2Road(h1, h2, h3, h4, -tb);
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

/// Find all fired roads enabled in `rs`, by taking all hit combinations from `vec`.
void FindFiredRoads(const SQHitVector* vec, const TrigRoadset* rs, const bool in_time, std::vector<int>* pos_top, std::vector<int>* pos_bot, std::vector<int>* neg_top, std::vector<int>* neg_bot)
{
  pos_top->clear();
  neg_top->clear();
  shared_ptr<SQHitVector> hv_h1t(UtilSQHit::FindFirstHits(vec, "H1T", in_time));
  shared_ptr<SQHitVector> hv_h2t(UtilSQHit::FindFirstHits(vec, "H2T", in_time));
  shared_ptr<SQHitVector> hv_h3t(UtilSQHit::FindFirstHits(vec, "H3T", in_time));
  shared_ptr<SQHitVector> hv_h4t(UtilSQHit::FindFirstHits(vec, "H4T", in_time));
  for (auto it1 = hv_h1t->begin(); it1 != hv_h1t->end(); it1++) {
  for (auto it2 = hv_h2t->begin(); it2 != hv_h2t->end(); it2++) {
  for (auto it3 = hv_h3t->begin(); it3 != hv_h3t->end(); it3++) {
  for (auto it4 = hv_h4t->begin(); it4 != hv_h4t->end(); it4++) {
    int road = Hodo2Road(
      (*it1)->get_element_id(), 
      (*it2)->get_element_id(), 
      (*it3)->get_element_id(), 
      (*it4)->get_element_id(), +1);
    if (rs->PosTop()->FindRoad(road)) pos_top->push_back(road);
    if (rs->NegTop()->FindRoad(road)) neg_top->push_back(road);
  }
  }
  }
  }
  pos_top->erase(std::unique(pos_top->begin(), pos_top->end()), pos_top->end());
  neg_top->erase(std::unique(neg_top->begin(), neg_top->end()), neg_top->end());
  
  pos_bot->clear();
  neg_bot->clear();
  shared_ptr<SQHitVector> hv_h1b(UtilSQHit::FindHits(vec, "H1B", in_time));
  shared_ptr<SQHitVector> hv_h2b(UtilSQHit::FindHits(vec, "H2B", in_time));
  shared_ptr<SQHitVector> hv_h3b(UtilSQHit::FindHits(vec, "H3B", in_time));
  shared_ptr<SQHitVector> hv_h4b(UtilSQHit::FindHits(vec, "H4B", in_time));
  for (auto it1 = hv_h1b->begin(); it1 != hv_h1b->end(); it1++) {
  for (auto it2 = hv_h2b->begin(); it2 != hv_h2b->end(); it2++) {
  for (auto it3 = hv_h3b->begin(); it3 != hv_h3b->end(); it3++) {
  for (auto it4 = hv_h4b->begin(); it4 != hv_h4b->end(); it4++) {
    int road = UtilTrigger::Hodo2Road(
      (*it1)->get_element_id(), 
      (*it2)->get_element_id(), 
      (*it3)->get_element_id(), 
      (*it4)->get_element_id(), -1);
    if (rs->PosBot()->FindRoad(road)) pos_bot->push_back(road);
    if (rs->NegBot()->FindRoad(road)) neg_bot->push_back(road);
  }
  }
  }
  }
  pos_bot->erase(std::unique(pos_bot->begin(), pos_bot->end()), pos_bot->end());
  neg_bot->erase(std::unique(neg_bot->begin(), neg_bot->end()), neg_bot->end());
}
  
}; // End of "namespace UtilTrigger"
