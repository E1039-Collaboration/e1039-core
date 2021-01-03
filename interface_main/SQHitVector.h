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
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SQHitVector* Clone() const {return NULL;}

  virtual bool   empty()                   const {return true;}
  virtual size_t  size()                   const {return 0;}
  virtual void   clear()                         {}

  virtual const SQHit* at(const size_t idkey) const {return nullptr;}
  virtual       SQHit* at(const size_t idkey) {return NULL;}
  virtual       void   push_back(const SQHit *hit) {}
  virtual       size_t erase(const size_t idkey) {return 0;}

  virtual ConstIter begin()                   const {return HitVector().end();}
  virtual ConstIter   end()                   const {return HitVector().end();}

  virtual Iter begin()                   {return HitVector().end();}
  virtual Iter   end()                   {return HitVector().end();}

protected:
  SQHitVector() {}

private:

  ClassDef(SQHitVector, 1);
};



#endif /* _H_SQHitVector_H_ */
