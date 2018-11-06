/*
 * SQSpill_v2.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#include "SQSpill_v2.h"
#include "SQStringMap_v1.h"
#include <limits>
#include <cmath>
using namespace std;

ClassImp(SQSpill_v2);

SQSpill_v2::SQSpill_v2() :
  _run_id(std::numeric_limits<int>::max()),
  _spill_id(std::numeric_limits<int>::max()),
  _target_pos(std::numeric_limits<short>::max()),
  _bos_coda_id (std::numeric_limits<int>::max()),
  _bos_vme_time(std::numeric_limits<int>::max()),
  _eos_coda_id (std::numeric_limits<int>::max()),
  _eos_vme_time(std::numeric_limits<int>::max()),
  _bos_scaler_list(new SQStringMap_v1()),
  _eos_scaler_list(new SQStringMap_v1()),
  _slow_cont_list(new SQStringMap_v1())
{}

SQSpill_v2::~SQSpill_v2()
{
  delete _bos_scaler_list;
  delete _eos_scaler_list;
  delete _slow_cont_list;
}

void SQSpill_v2::identify(ostream& os) const {
  os << "---SQSpill_v2--------------------" << endl;
  os << " runID: " << get_run_id() << endl;
  os << " spillID: " << get_spill_id() << endl;
  os << " liveProton: " << get_target_pos() << endl;
  os << "---------------------------------" << endl;

  return;
}

int SQSpill_v2::isValid() const {
  if (_run_id == std::numeric_limits<int>::max()) return 0;
  if (_spill_id == std::numeric_limits<int>::max()) return 0;
  if (_target_pos == std::numeric_limits<short>::max()) return 0;
  return 1;
}


