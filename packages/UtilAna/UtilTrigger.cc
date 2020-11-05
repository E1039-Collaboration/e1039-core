#include <cmath>
using namespace std;
namespace UtilTrigger {

/// Convert a set of hodo IDs to a roadset ID.
/**
 * Note that the max hodo IDs are 23 (H1), 16 (H2), 16 (H3) and 16 (H4).
 */
int Hodo2Road(const int h1, const int h2, const int h3, const int h4, const int tb)
{
  return ((h1-1)*16*16*16 + (h2-1)*16*16 + (h3-1)*16 + (h4-1)) * (tb>0?+1:-1);
}

/// Convert a roadset ID to a set of hodo IDs.
void Road2Hodo(const int road, int& h1, int& h2, int& h3, int& h4, int& tb)
{
  int rr = abs(road) - 1;
  h1 = 1 +  rr/16/16/16;
  h2 = 1 + (rr/16/16)%16;
  h3 = 1 + (rr/16)   %16;
  h4 = 1 +  rr       %16;
  tb = road>0 ? +1 : -1;
}

}; // End of "namespace UtilTrigger"
