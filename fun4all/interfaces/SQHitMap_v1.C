/*
 * SQHitMap_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "SQHitMap_v1.h"

#include "SQHit.h"

#include <map>

using namespace std;

ClassImp(SQHitMap_v1)

SQHitMap_v1::SQHitMap_v1()
: _map() {
}

SQHitMap_v1::SQHitMap_v1(const SQHitMap_v1& hitmap)
  : _map() {
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
    const SQHit *hit = iter->second;
    _map.insert(make_pair(hit->get_hit_id(),hit->Clone()));
  }
}

SQHitMap_v1& SQHitMap_v1::operator=(const SQHitMap_v1& hitmap) {
  Reset();
  for (ConstIter iter = hitmap.begin();
       iter != hitmap.end();
       ++iter) {
    const SQHit *hit = iter->second;
    _map.insert(make_pair(hit->get_hit_id(),hit->Clone()));
  }
  return *this;
}

SQHitMap_v1::~SQHitMap_v1() {
  Reset();
}

void SQHitMap_v1::Reset() {
  for (Iter iter = _map.begin();
       iter != _map.end();
       ++iter) {
    SQHit *hit = iter->second;
    delete hit;
  }
  _map.clear();
}

void SQHitMap_v1::identify(ostream& os) const {
  os << "SQHitMap_v1: size = " << _map.size() << endl;
  return;
}

const SQHit* SQHitMap_v1::get(unsigned int id) const {
  ConstIter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SQHit* SQHitMap_v1::get(unsigned int id) {
  Iter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

SQHit* SQHitMap_v1::insert(const SQHit *hit) {
	int index = hit->get_hit_id();
  _map.insert(make_pair( index , hit->Clone() ));
  return _map[index];
}


