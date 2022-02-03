/*
 * SQHitVector_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQHitVector_v1_H_
#define _H_SQHitVector_v1_H_

#include <phool/PHObject.h>
#include <map>
#include <iostream>

#include "SQHit.h"
#include "SQHitVector.h"

class SQHitVector_v1 : public SQHitVector {

public:

  SQHitVector_v1();
  SQHitVector_v1(const SQHitVector_v1& hitmap);
  SQHitVector_v1& operator=(const SQHitVector_v1& hitmap);
  virtual ~SQHitVector_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SQHitVector* Clone() const {return new SQHitVector_v1(*this);}

  bool   empty()                   const {return _vector.empty();}
  size_t  size()                   const {return _vector.size();}
  void   clear()                         {Reset();}

  const SQHit* at(const size_t idkey) const;
  SQHit*       at(const size_t idkey);
  void         push_back(const SQHit *hit);
  size_t       erase(const size_t idkey) {
	  delete _vector[idkey];
	  _vector.erase(_vector.begin() + idkey);
	  return _vector.size();
	}
  Iter erase(Iter pos) { return _vector.erase(pos); }

  ConstIter begin()                   const {return _vector.begin();}
  ConstIter   end()                   const {return _vector.end();}

  Iter begin()                   {return _vector.begin();}
  Iter   end()                   {return _vector.end();}

private:
  HitVector _vector;

  ClassDef(SQHitVector_v1, 1);
};

#endif
