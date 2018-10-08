/*
 * SQSlowCont_v1.C
 */
#include "SQSlowCont_v1.h"

#include <limits>
#include <cmath>

//#include <TMatrixF.h>

using namespace std;

ClassImp(SQSlowCont_v1);

SQSlowCont_v1::SQSlowCont_v1() : _time_stamp(""), _name(""), _value(""), _type("")
{}

void SQSlowCont_v1::identify(ostream& os) const {
  os << "---SQSlowCont_v1--------------------" << endl;
  os << "  name: " << get_name() << endl;
  os << "---------------------------------" << endl;
  return;
}

int SQSlowCont_v1::isValid() const {
  return (_name.length() > 0) ? 1 : 0;
}
