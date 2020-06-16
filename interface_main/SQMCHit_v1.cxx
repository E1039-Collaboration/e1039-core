/*
 * SQMCHit_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */
#include "SQMCHit_v1.h"

#include <limits>
#include <cmath>

using namespace std;

ClassImp(SQMCHit_v1);

SQMCHit_v1::SQMCHit_v1()
  : _track_id(std::numeric_limits<int>::max()),
    _g4hit_id(std::numeric_limits<PHG4HitDefs_keytype>::max())
{}

void SQMCHit_v1::identify(ostream& os) const {
  os << "---SQMCHit_v1--------------------" << endl;
  os << "track_id: " << get_track_id() << endl;
  os << "hit_id: " << get_hit_id() << endl;
  os << "truth_pos: (" << _truth_x << ", " << _truth_y << ", " << _truth_z << ")" << endl;
  os << "truth_mom: (" << _truth_px << ", " << _truth_py << ", " << _truth_pz << ")" << endl;

  SQHit_v1::identify(os);

  return;
}

int SQMCHit_v1::isValid() const {
  if (_track_id == std::numeric_limits<int>::max()) return 0;
  return SQHit_v1::isValid();
}


