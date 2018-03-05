/*
 * SQSpill_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#include "SQSpill_v1.h"

#include <limits>
#include <cmath>

//#include <TMatrixF.h>

using namespace std;

ClassImp(SQSpill_v1);

SQSpill_v1::SQSpill_v1() :
_run_id(std::numeric_limits<int>::max()),
_spill_id(std::numeric_limits<int>::max()),
_target_pos(std::numeric_limits<short>::max())
{}

void SQSpill_v1::identify(ostream& os) const {
  os << "---SQSpill_v1--------------------" << endl;
  os << " runID: " << get_run_id() << endl;
  os << " spillID: " << get_spill_id() << endl;
  os << " liveProton: " << get_target_pos() << endl;
  os << "---------------------------------" << endl;

  return;
}

int SQSpill_v1::isValid() const {
  if (_run_id == std::numeric_limits<int>::max()) return 0;
  if (_spill_id == std::numeric_limits<int>::max()) return 0;
  if (_target_pos == std::numeric_limits<short>::max()) return 0;
  return 1;
}


