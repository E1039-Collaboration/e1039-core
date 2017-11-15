/*
 * SQHit_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#include "SQHit_v1.h"

#include <limits>
#include <cmath>

//#include <TMatrixF.h>

using namespace std;

ClassImp(SQHit_v1);

SQHit_v1::SQHit_v1()
  : _hit_id(std::numeric_limits<int>::max()),
	_detector_id(std::numeric_limits<short>::max()),
	_element_id(std::numeric_limits<short>::max()),
	_tdc_time(NAN),
	_drift_distance(NAN),
	_pos(NAN),
	_flag(0)
{}

void SQHit_v1::identify(ostream& os) const {
  os << "---SQHit_v1--------------------" << endl;
  os << " hitID: " << get_hit_id() << endl;
  os << " detectorID: " << get_detector_id() << " elementID: "<< get_element_id() << endl;
  os
  << " tdcTime: " << get_tdc_time()
  << " driftDistance: " << get_drift_distance()
  << " pos: " << get_pos()
  << endl;
  os
  << " inTime: " << is_in_time()
  << " masked: " << is_hodo_mask()
  << " is_trigger_mask: " << is_trigger_mask()
  << endl;
  os << "---------------------------------" << endl;

  return;
}

int SQHit_v1::isValid() const {
  if (_hit_id == std::numeric_limits<int>::max()) return 0;
  if (_detector_id == std::numeric_limits<short>::max()) return 0;
  if (_element_id == std::numeric_limits<short>::max()) return 0;
  if (isnan(_tdc_time)) return 0;
  if (isnan(_drift_distance)) return 0;
  if (isnan(_pos)) return 0;
  return 1;
}


