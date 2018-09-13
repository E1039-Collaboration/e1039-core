#include <time.h>
int time_(void)
{
  time_t now;
  now = time((time_t *)0);
  return((int) now);
}
