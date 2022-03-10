#include "SQHardEvent.h"
using namespace std;

ClassImp(SQHardEvent)

void SQHardEvent::identify(std::ostream& os) const
{
  os << "---SQHardEvent::identify: abstract base-------------------" << endl;
}
