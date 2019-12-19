#ifndef _SQ_TRACK_VECTOR_V1__H_
#define _SQ_TRACK_VECTOR_V1__H_
#include "SQTrackVector.h"

class SQTrackVector_v1: public SQTrackVector {
 public:
  SQTrackVector_v1();
  SQTrackVector_v1(const SQTrackVector_v1& obj);
  SQTrackVector_v1& operator=(const SQTrackVector_v1& obj);
  virtual ~SQTrackVector_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const { return 1; }
  SQTrackVector* Clone() const { return new SQTrackVector_v1(*this); }

  ConstIter begin() const { return _vector.begin(); }
  ConstIter end  () const { return _vector.end  (); }
  Iter      begin()       { return _vector.begin(); }
  Iter      end  ()       { return _vector.end  (); }
  bool      empty() const { return _vector.empty(); }
  size_t    size () const { return _vector.size (); }
  void      clear() { Reset(); }

  const SQTrack* at(const size_t id) const;
  SQTrack*       at(const size_t id);
  void   push_back(const SQTrack *trk);
  size_t erase(const size_t id);

 protected:
  Vector _vector;

  ClassDef(SQTrackVector_v1, 1);
};

#endif // _SQ_TRACK_VECTOR_V1__H_
