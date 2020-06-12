/*
 * SQCalHit_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */
#include "SQCalHit_v1.h"

#include <limits>
#include <cmath>

using namespace std;

ClassImp(SQCalHit_v1);

SQCalHit_v1::SQCalHit_v1(): 
  _track_id(std::numeric_limits<int>::max()),
  _shower_id(std::numeric_limits<int>::max()),
	_g4hit_id(std::numeric_limits<PHG4HitDefs_keytype>::max()),
  _edep(0.)
{
  _truth_cells.clear();
}

void SQCalHit_v1::identify(ostream& os) const {
  os << "---SQCalHit_v1--------------------" << endl;
  os << "track_id: " << get_track_id() << endl;
  os << "hit_id: " << get_hit_id() << endl;
  os << "shower_id: 0x" << hex << get_hit_id() << endl;
  os << "edep: " << _edep << endl;
  os << "Number of cells: " << get_n_cells() << endl;
  os << "truth_pos: (" << _truth_x << ", " << _truth_y << ", " << _truth_z << ")" << endl;
  os << "truth_mom: (" << _truth_px << ", " << _truth_py << ", " << _truth_pz << ")" << endl;

  SQHit_v1::identify(os);

  return;
}

int SQCalHit_v1::isValid() const {
  if (_track_id == std::numeric_limits<int>::max()) return 0;
  return SQHit_v1::isValid();
}

float SQCalHit_v1::get_cell(short cellID)
{
  if(_truth_cells.find(cellID) != _truth_cells.end()) return _truth_cells[cellID];
  return -1.;
}

void SQCalHit_v1::add_cell(short cellID, float edep)
{
  if(_truth_cells.find(cellID) != _truth_cells.end())
  {
    _truth_cells[cellID] += edep;
  }
  else
  {
    _truth_cells[cellID] = edep;
  }
  _edep += edep;
}