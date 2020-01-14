#ifndef _SQ_DIMUON_VECTOR_V1__H_
#define _SQ_DIMUON_VECTOR_V1__H_
#include "SQDimuonVector.h"

class SQDimuonVector_v1: public SQDimuonVector {
 public:
  SQDimuonVector_v1();
  SQDimuonVector_v1(const SQDimuonVector_v1& obj);
  SQDimuonVector_v1& operator=(const SQDimuonVector_v1& obj);
  virtual ~SQDimuonVector_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const { return 1; }
  SQDimuonVector* Clone() const { return new SQDimuonVector_v1(*this); }

  ConstIter begin() const { return _vector.begin(); }
  ConstIter end  () const { return _vector.end  (); }
  Iter      begin()       { return _vector.begin(); }
  Iter      end  ()       { return _vector.end  (); }
  bool      empty() const { return _vector.empty(); }
  size_t    size () const { return _vector.size (); }
  void      clear() { Reset(); }

  const SQDimuon* at(const size_t id) const;
  SQDimuon*       at(const size_t id);
  void   push_back(const SQDimuon *dim);
  size_t erase(const size_t id);

 protected:
  Vector _vector;

  ClassDef(SQDimuonVector_v1, 1);
};

#endif // _SQ_DIMUON_VECTOR_V1__H_
