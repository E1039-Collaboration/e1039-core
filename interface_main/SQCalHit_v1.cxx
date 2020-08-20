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
  _hit_id(std::numeric_limits<int>::max()),
  _detector_id(std::numeric_limits<int>::max()),
	_element_id(std::numeric_limits<int>::max())
{
  _cells.clear();
}

void SQCalHit_v1::identify(ostream& os) const {
  os << "---SQCalHit_v1--------------------" << endl;
  os << " hitID: " << get_hit_id() << endl;
  os << " detectorID: " << get_detector_id() << " elementID: "<< get_element_id() << " edep: "<< get_edep() << endl;
  os << "---------------------------------" << endl;

  return;
}

int SQCalHit_v1::isValid() const {
  if (_hit_id == std::numeric_limits<int>::max()) return 0;
  return 1;
}

float SQCalHit_v1::get_cell(short cellID) const
{
  auto it = _cells.find(cellID);
  if(it != _cells.end()) return it->second;
  return -1.;
}

void SQCalHit_v1::add_cell(short cellID, float edep)
{
  if(_cells.find(cellID) != _cells.end()) 
    _cells[cellID] += edep;
  else
    _cells[cellID] = edep;
    
  _edep += edep;
}