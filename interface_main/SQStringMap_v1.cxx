/*
 * SQStringMap_v1.C
 */
#include "SQStringMap_v1.h"
using namespace std;

ClassImp(SQStringMap_v1)

SQStringMap_v1::SQStringMap_v1() : _map()
{}

SQStringMap_v1::SQStringMap_v1(const SQStringMap_v1& map) : _map()
{
  for (ConstIter iter = map.begin(); iter != map.end(); ++iter) {
    _map.insert(make_pair(iter->first, iter->second->clone()));
  }
}

SQStringMap_v1& SQStringMap_v1::operator=(const SQStringMap_v1& map)
{
  Reset();
  for (ConstIter iter = map.begin(); iter != map.end(); ++iter) {
    _map.insert(make_pair(iter->first, iter->second->clone()));
  }
  return *this;
}

SQStringMap_v1::~SQStringMap_v1() {
  Reset();
}

void SQStringMap_v1::Reset() {
  for (Iter iter = _map.begin(); iter != _map.end(); ++iter) {
    delete iter->second;
  }
  _map.clear();
}

void SQStringMap_v1::identify(ostream& os) const {
  os << "SQStringMap_v1: size = " << _map.size() << endl;
  return;
}

const PHObject* SQStringMap_v1::get(std::string key) const {
  ConstIter iter = _map.find(key);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

PHObject* SQStringMap_v1::get(std::string key) {
  Iter iter = _map.find(key);
  if (iter == _map.end()) return NULL;
  return iter->second;
}

PHObject* SQStringMap_v1::insert(const std::string key, const PHObject *item) {
  _map.insert(make_pair(key, item->clone() ));
  return _map[key];
}
