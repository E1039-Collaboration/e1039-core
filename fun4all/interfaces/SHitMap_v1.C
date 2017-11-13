/*
 * SHitMap_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "SHitMap_v1.h"

#include "SHit.h"

#include <map>

using namespace std;

ClassImp(SHitMap_v1)

SHitMap_v1::SHitMap_v1()
: _map() {
}

SHitMap_v1::SHitMap_v1(const SHitMap_v1& hitmap)
  : _map() {
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
    const SHit *hit = iter->second;
    _map.insert(make_pair(hit->get_hit_id(),hit->Clone()));
  }
}

SHitMap_v1& SHitMap_v1::operator=(const SHitMap_v1& hitmap) {
  Reset();
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
    const SHit *hit = iter->second;
    _map.insert(make_pair(hit->get_hit_id(),hit->Clone()));
  }
  return *this;
}

SHitMap_v1::~SHitMap_v1() {
  Reset();
}

void SHitMap_v1::Reset() {
  for (Iter iter = _map.begin();
       iter != _map.end();
       ++iter) {
    SHit *hit = iter->second;
    delete hit;
  }
  _map.clear();
}

void SHitMap_v1::identify(ostream& os) const {
  os << "SHitMap_v1: size = " << _map.size() << endl;
  return;
}

const SHit* SHitMap_v1::get(unsigned int id) const {
  ConstIter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SHit* SHitMap_v1::get(unsigned int id) {
  Iter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SHit* SHitMap_v1::insert(const SHit *hit) {
  unsigned int index = 0;
  if (!_map.empty()) index = _map.rbegin()->first + 1;
  _map.insert(make_pair( index , hit->Clone() ));
  _map[index]->set_hit_id(index);
  return _map[index];
}


