/*
 * SQIntMap_v1.C
 */
#include "SQIntMap_v1.h"
using namespace std;

ClassImp(SQIntMap_v1)

SQIntMap_v1::SQIntMap_v1() : _map()
{}

SQIntMap_v1::SQIntMap_v1(const SQIntMap_v1& map) : _map()
{
  for (ConstIter iter = map.begin(); iter != map.end(); ++iter) {
    _map.insert(make_pair(iter->first, iter->second->clone()));
  }
}

SQIntMap_v1& SQIntMap_v1::operator=(const SQIntMap_v1& map)
{
  Reset();
  for (ConstIter iter = map.begin(); iter != map.end(); ++iter) {
    _map.insert(make_pair(iter->first, iter->second->clone()));
  }
  return *this;
}

SQIntMap_v1::~SQIntMap_v1() {
  Reset();
}

void SQIntMap_v1::Reset() {
  for (Iter iter = _map.begin(); iter != _map.end(); ++iter) {
    delete iter->second;
  }
  _map.clear();
}

void SQIntMap_v1::identify(ostream& os) const {
  os << "SQIntMap_v1: size = " << _map.size() << endl;
  return;
}

const PHObject* SQIntMap_v1::get(unsigned int id) const {
  ConstIter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

PHObject* SQIntMap_v1::get(unsigned int id) {
  Iter iter = _map.find(id);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

PHObject* SQIntMap_v1::insert(const unsigned int id, const PHObject *item) {
  _map.insert(make_pair(id, item->clone() ));
  return _map[id];
}
