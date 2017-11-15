/*
 * SQSpillMap_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "SQSpillMap_v1.h"

#include "SQSpill.h"

#include <map>

using namespace std;

ClassImp(SQSpillMap_v1)

SQSpillMap_v1::SQSpillMap_v1()
: _map() {
}

SQSpillMap_v1::SQSpillMap_v1(const SQSpillMap_v1& hitmap)
  : _map() {
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
    const SQSpill *spill = iter->second;
    _map.insert(make_pair(spill->get_spill_id(),spill->Clone()));
  }
}

SQSpillMap_v1& SQSpillMap_v1::operator=(const SQSpillMap_v1& hitmap) {
  Reset();
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
	  const SQSpill *spill = iter->second;
	  _map.insert(make_pair(spill->get_spill_id(),spill->Clone()));
  }
  return *this;
}

SQSpillMap_v1::~SQSpillMap_v1() {
  Reset();
}

void SQSpillMap_v1::Reset() {
  for (Iter iter = _map.begin();
       iter != _map.end();
       ++iter) {
    SQSpill *hit = iter->second;
    delete hit;
  }
  _map.clear();
}

void SQSpillMap_v1::identify(ostream& os) const {
  os << "SQSpillMap_v1: size = " << _map.size() << endl;
  return;
}

const SQSpill* SQSpillMap_v1::get(unsigned int id) const {
  ConstIter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SQSpill* SQSpillMap_v1::get(unsigned int id) {
  Iter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SQSpill* SQSpillMap_v1::insert(const SQSpill *hit) {
  unsigned int index = 0;
  if (!_map.empty()) index = _map.rbegin()->first + 1;
  _map.insert(make_pair( index , hit->Clone() ));
  return _map[index];
}


