/*
 * HitVector.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQHitVector_H_
#define _H_SQHitVector_H_


#include <phool/PHObject.h>

#include <vector>
#include <iostream>

#include "SQHit.h"

/// An SQ interface class to hold a list of SQHit objects.
/**
 * Below is the standard way to find this object in "topNode" and loop over each hit;
 * @code
 * SQHitVector* hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
 * for (SQHitVector::ConstIter it = hit_vec->begin(); it != hit_vec->end(); it++) {
 *   SQHit* hit = *it;
 *   int det_id = hit->get_detector_id();
 * }
 * @endcode
 *
 * You can use utility functions defined in UtilSQHit for more complicated manipulations.
 */
class SQHitVector : public PHObject {

public:

  typedef std::vector<SQHit*>       HitVector;
  typedef std::vector<SQHit*>::const_iterator ConstIter;
  typedef std::vector<SQHit*>::iterator            Iter;

  virtual ~SQHitVector() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SQHitVector base class" << std::endl;
  }
  virtual void Reset() = 0;
  virtual int  isValid() const = 0;
  virtual SQHitVector* Clone() const = 0;

  virtual bool   empty() const = 0;
  virtual size_t  size() const = 0;
  virtual void   clear() = 0;

  virtual const SQHit* at(const size_t idkey) const = 0;
  virtual       SQHit* at(const size_t idkey) = 0;
  virtual       void   push_back(const SQHit *hit) = 0;
  virtual       size_t erase(const size_t idkey) = 0;
  virtual       Iter   erase(Iter pos) = 0;

  virtual ConstIter begin() const = 0;
  virtual ConstIter   end() const = 0;

  virtual Iter begin() = 0;
  virtual Iter   end() = 0;

protected:
  SQHitVector() {}

private:

  ClassDef(SQHitVector, 1);
};



#endif /* _H_SQHitVector_H_ */
