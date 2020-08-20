#include "SQCalMCHit_v1.h"

#include <limits>
#include <cmath>

using namespace std;

ClassImp(SQCalMCHit_v1);

SQCalMCHit_v1::SQCalMCHit_v1(): 
  _track_id(std::numeric_limits<int>::max()),
  _shower_id(std::numeric_limits<int>::max()),
	_g4hit_id(std::numeric_limits<PHG4HitDefs::keytype>::max())
{
}

void SQCalMCHit_v1::identify(ostream& os) const {
  os << "---SQCalMCHit_v1--------------------" << endl;
  os << "track_id: " << get_track_id() << endl;
  os << "hit_id: " << get_hit_id() << endl;
  os << "shower_id: 0x" << hex << get_hit_id() << dec << endl;
  os << "truth_pos: (" << _truth_x << ", "  << _truth_y << ", "  << _truth_z << ")" << endl;
  os << "truth_mom: (" << _truth_px << ", " << _truth_py << ", " << _truth_pz << ")" << endl;

  SQCalHit_v1::identify(os);

  return;
}

int SQCalMCHit_v1::isValid() const {
  if (_track_id == std::numeric_limits<int>::max()) return 0;
  return SQCalHit_v1::isValid();
}
