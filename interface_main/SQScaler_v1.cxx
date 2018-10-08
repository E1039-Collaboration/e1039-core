/*
 * SQScaler_v1.C
 */
#include "SQScaler_v1.h"

#include <limits>
#include <cmath>

//#include <TMatrixF.h>

using namespace std;

ClassImp(SQScaler_v1);

SQScaler_v1::SQScaler_v1() : _type(UNDEF), _name(""), _count(0)
{}

void SQScaler_v1::identify(ostream& os) const {
  os << "---SQScaler_v1--------------------" << endl;
  os << "  type: " << get_type() << endl;
  os << "  name: " << get_name() << endl;
  os << " count: " << get_count() << endl;
  os << "---------------------------------" << endl;
  return;
}

int SQScaler_v1::isValid() const {
  return (_type != UNDEF && _name.length() > 0) ? 1 : 0;
}
