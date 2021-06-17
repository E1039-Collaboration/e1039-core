#ifndef _UTIL_TRIGGER__H_
#define _UTIL_TRIGGER__H_
//#include <vector>
class SQHitVector;

namespace UtilTrigger {
  int  Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb);
  void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb);
  int  ExtractRoadID(const SQHitVector* vec);
}; // namespace UtilTrigger

#endif /* _UTIL_TRIGGER__H_ */
