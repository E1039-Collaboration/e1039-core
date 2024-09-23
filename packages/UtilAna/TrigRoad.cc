#include <iostream>
#include <sstream>
#include <cstring>
#include "TrigRoad.h"
using namespace std;
namespace UtilTrigger {
  
TrigRoad::TrigRoad()
  : road_id(0)
  , charge (0)
  , H1X    (0)
  , H2X    (0)
  , H3X    (0)
  , H4X    (0)
{
  ;
}

void TrigRoad::Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb)
{
  int rr = abs(road) - 1;
  h1 = 1 + (rr / 4096);
  h2 = 1 + (rr /  256) % 16;
  h3 = 1 + (rr /   16) % 16;
  h4 = 1 +  rr         % 16;
  tb = road / abs(road);
}

int TrigRoad::Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb)
{
  int rr = 4096*(h1-1) + 256*(h2-1) + 16*(h3-1) + h4;
  if (tb < 0) rr *= -1;
  return rr;
}

std::string TrigRoad::str(const int level) const
{
  ostringstream oss;
  oss << road_id;
  if (level > 0) oss << "[" << H1X << "," << H2X << "," << H3X << "," << H4X << "]";
  if (level > 1) oss << (charge > 0 ? '+' : '-');
  return oss.str();
}
  
}; // End of "namespace UtilTrigger"
