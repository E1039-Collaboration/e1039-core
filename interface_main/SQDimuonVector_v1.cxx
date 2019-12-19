#include "SQDimuonVector_v1.h"
using namespace std;
ClassImp(SQDimuonVector_v1)

SQDimuonVector_v1::SQDimuonVector_v1()
{
  ;
}

SQDimuonVector_v1::SQDimuonVector_v1(const SQDimuonVector_v1& obj)
{
  for (ConstIter iter = obj.begin(); iter != obj.end(); ++iter) {
    _vector.push_back((*iter)->Clone());
  }
}

SQDimuonVector_v1& SQDimuonVector_v1::operator=(const SQDimuonVector_v1& obj)
{
  Reset();
  for (ConstIter iter = obj.begin(); iter != obj.end(); ++iter) {
    _vector.push_back((*iter)->Clone());
  }
  return *this;
}

SQDimuonVector_v1::~SQDimuonVector_v1()
{
  Reset();
}

void SQDimuonVector_v1::Reset() 
{
  for (Iter iter = _vector.begin(); iter != _vector.end(); ++iter) delete *iter;
  _vector.clear();
}

void SQDimuonVector_v1::identify(ostream& os) const
{
  os << "SQDimuonVector_v1: size = " << _vector.size() << endl;
}

const SQDimuon* SQDimuonVector_v1::at(const size_t id) const
{
  if (id >= _vector.size()) return nullptr;
  return _vector[id];
}

SQDimuon* SQDimuonVector_v1::at(const size_t id)
{
  if (id >= _vector.size()) return nullptr;
  return _vector[id];
}

void SQDimuonVector_v1::push_back(const SQDimuon *dim)
{
  _vector.push_back(dim->Clone());
}

size_t SQDimuonVector_v1::erase(const size_t id)
{
  delete _vector[id];
  _vector.erase(_vector.begin() + id);
  return _vector.size();
}
