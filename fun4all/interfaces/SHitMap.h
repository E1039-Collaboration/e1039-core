/*
 * HitMap.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SHitMap_H_
#define _H_SHitMap_H_


#include "SHit.h"

#include <phool/PHObject.h>
#include <map>
#include <iostream>

class SHitMap : public PHObject {

public:

  typedef std::map<unsigned int, SHit*> HitMap;
  typedef std::map<unsigned int, SHit*>::const_iterator ConstIter;
  typedef std::map<unsigned int, SHit*>::iterator            Iter;

  virtual ~SHitMap() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SHitMap base class" << std::endl;
  }
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SHitMap* Clone() const {return NULL;}

  virtual bool   empty()                   const {return true;}
  virtual size_t  size()                   const {return 0;}
  virtual size_t count(unsigned int idkey) const {return 0;}
  virtual void   clear()                         {}

  virtual const SHit* get(unsigned int idkey) const {return NULL;}
  virtual       SHit* get(unsigned int idkey) {return NULL;}
  virtual       SHit* insert(const SHit *hit) {return NULL;}
  virtual       size_t   erase(unsigned int idkey) {return 0;}

  virtual ConstIter begin()                   const {return HitMap().end();}
  virtual ConstIter  find(unsigned int idkey) const {return HitMap().end();}
  virtual ConstIter   end()                   const {return HitMap().end();}

  virtual Iter begin()                   {return HitMap().end();}
  virtual Iter  find(unsigned int idkey) {return HitMap().end();}
  virtual Iter   end()                   {return HitMap().end();}

protected:
  SHitMap() {}

private:

  ClassDef(SHitMap, 1);
};



#endif /* _H_SHitMap_H_ */
