#ifndef _TRIG_ROAD__H_
#define _TRIG_ROAD__H_
#include <string>

namespace UtilTrigger {
  
class TrigRoad {
 public:
  int road_id;
  int charge;
  int H1X;
  int H2X;
  int H3X;
  int H4X;
  
  TrigRoad();
  virtual ~TrigRoad() {;}
  
  static void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb);
  static int  Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb);
  
  std::string str(const int level=0) const;
};
  
}; // namespace UtilTrigger

#endif // _TRIG_ROAD__H_
