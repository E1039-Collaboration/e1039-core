/*
 * SHit_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */
#include "SHit_v1.h"

#include <cmath>

//#include <TMatrixF.h>

using namespace std;

ClassImp(SHit_v1);

SHit_v1::SHit_v1()
  : _hit_id(0xFFFFFFFF),
	_detector_name(""),
	_element_id(INT_MAX),
	_drift_distance(NAN)
{}

void SHit_v1::identify(ostream& os) const {
  os << "---SHit_v1--------------------" << endl;
  os << "hitID: " << get_id() << endl;
  os << "detectorName: " << get_detector_name() << " elementID: "<< get_element_id() << endl;
  os << "driftDistance: " << get_drift_distance() << endl;
  os << "---------------------------------" << endl;

  return;
}

int SHit_v1::isValid() const {
  if (_hit_id == 0xFFFFFFFF) return 0;
  if (_detector_name.empty()) return 0;
  if (_element_id == INT_MAX) return 0;
  if (isnan(_drift_distance)) return 0;
  return 1;
}


