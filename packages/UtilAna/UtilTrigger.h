#ifndef _UTIL_TRIGGER__H_
#define _UTIL_TRIGGER__H_
//#include <vector>
class SQHitVector;

namespace UtilTrigger {
  class TrigRoadset;
  int  Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb);
  void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb);
  int FlipRoadLeftRight(const int road);
  int FlipRoadTopBottom(const int road);
  int  ExtractRoadID(const SQHitVector* vec);
  void FindFiredRoads(const SQHitVector* vec, const TrigRoadset* rs, const bool in_time, std::vector<int>* pos_top, std::vector<int>* pos_bot, std::vector<int>* neg_top, std::vector<int>* neg_bot);

}; // namespace UtilTrigger

#endif /* _UTIL_TRIGGER__H_ */
