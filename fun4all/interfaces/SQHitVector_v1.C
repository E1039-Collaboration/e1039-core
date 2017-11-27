/*
 * SQHitVector_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */


#include "SQHitVector_v1.h"

#include "SQHit.h"

#include <map>

using namespace std;

ClassImp(SQHitVector_v1)

SQHitVector_v1::SQHitVector_v1()
: _vector() {
}

SQHitVector_v1::SQHitVector_v1(const SQHitVector_v1& hitvector)
  : _vector() {
  for (ConstIter iter = hitvector.begin();
       iter != hitvector.end();
       ++iter) {
    const SQHit *hit = *iter;
    _vector.push_back(hit->Clone());
  }
}

SQHitVector_v1& SQHitVector_v1::operator=(const SQHitVector_v1& hitvector) {
  Reset();
  for (ConstIter iter = hitvector.begin();
       iter != hitvector.end();
       ++iter) {
    const SQHit *hit = *iter;
    _vector.push_back(hit->Clone());
  }
  return *this;
}

SQHitVector_v1::~SQHitVector_v1() {
  Reset();
}

void SQHitVector_v1::Reset() {
  for (Iter iter = _vector.begin();
       iter != _vector.end();
       ++iter) {
    SQHit *hit = *iter;
    delete hit;
  }
  _vector.clear();
}

void SQHitVector_v1::identify(ostream& os) const {
  os << "SQHitVector_v1: size = " << _vector.size() << endl;
  return;
}

const SQHit* SQHitVector_v1::at(const size_t id) const {
  if(id>= size()) return nullptr;
  return _vector[id];
}

SQHit* SQHitVector_v1::at(const size_t id) {
  if(id>= size()) return nullptr;
  return _vector[id];
}

void SQHitVector_v1::push_back(const SQHit *hit) {
  _vector.push_back(hit->Clone());
}


