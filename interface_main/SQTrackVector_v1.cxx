#include "SQTrackVector_v1.h"
using namespace std;
ClassImp(SQTrackVector_v1)

SQTrackVector_v1::SQTrackVector_v1()
{
  ;
}

SQTrackVector_v1::SQTrackVector_v1(const SQTrackVector_v1& obj)
{
  for (ConstIter iter = obj.begin(); iter != obj.end(); ++iter) {
    _vector.push_back((*iter)->Clone());
  }
}

SQTrackVector_v1& SQTrackVector_v1::operator=(const SQTrackVector_v1& obj)
{
  Reset();
  for (ConstIter iter = obj.begin(); iter != obj.end(); ++iter) {
    _vector.push_back((*iter)->Clone());
  }
  return *this;
}

SQTrackVector_v1::~SQTrackVector_v1()
{
  Reset();
}

void SQTrackVector_v1::Reset() 
{
  for (Iter iter = _vector.begin(); iter != _vector.end(); ++iter) delete *iter;
  _vector.clear();
}

void SQTrackVector_v1::identify(ostream& os) const
{
  os << "SQTrackVector_v1: size = " << _vector.size() << endl;
}

const SQTrack* SQTrackVector_v1::at(const size_t id) const
{
  if (id >= _vector.size()) return nullptr;
  return _vector[id];
}

SQTrack* SQTrackVector_v1::at(const size_t id)
{
  if (id >= _vector.size()) return nullptr;
  return _vector[id];
}

void SQTrackVector_v1::push_back(const SQTrack *trk)
{
  _vector.push_back(trk->Clone());
}

size_t SQTrackVector_v1::erase(const size_t id)
{
  delete _vector[id];
  _vector.erase(_vector.begin() + id);
  return _vector.size();
}
