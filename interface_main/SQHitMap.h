/*
 * HitMap.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQHitMap_H_
#define _H_SQHitMap_H_


#include <phool/PHObject.h>

#include <map>
#include <iostream>

#include "SQHit.h"

/// An SQ interface class to hold a list of SQHit objects as std::map.
/**
 * This class is _not_ being used.  Instead the standard list of SQHit is SQHitVector.  This class could be deleted in future.
 */
class SQHitMap : public PHObject {

public:

  typedef std::map<unsigned int, SQHit*> HitMap;
  typedef std::map<unsigned int, SQHit*>::const_iterator ConstIter;
  typedef std::map<unsigned int, SQHit*>::iterator            Iter;

  virtual ~SQHitMap() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SQHitMap base class" << std::endl;
  }
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SQHitMap* Clone() const {return NULL;}

  virtual bool   empty()                   const {return true;}
  virtual size_t  size()                   const {return 0;}
  virtual size_t count(unsigned int idkey) const {return 0;}
  virtual void   clear()                         {}

  virtual const SQHit* get(unsigned int idkey) const {return NULL;}
  virtual       SQHit* get(unsigned int idkey) {return NULL;}
  virtual       SQHit* insert(const SQHit *hit) {return NULL;}
  virtual       size_t   erase(unsigned int idkey) {return 0;}

  virtual ConstIter begin()                   const {return HitMap().end();}
  virtual ConstIter  find(unsigned int idkey) const {return HitMap().end();}
  virtual ConstIter   end()                   const {return HitMap().end();}

  virtual Iter begin()                   {return HitMap().end();}
  virtual Iter  find(unsigned int idkey) {return HitMap().end();}
  virtual Iter   end()                   {return HitMap().end();}

protected:
  SQHitMap() {}

private:

  ClassDef(SQHitMap, 1);
};



#endif /* _H_SQHitMap_H_ */
