/*
 * SpillMap.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQSpillMap_H_
#define _H_SQSpillMap_H_


#include "SQSpill.h"

#include <phool/PHObject.h>
#include <map>
#include <iostream>

class SQSpillMap : public PHObject {

public:

  typedef std::map<unsigned int, SQSpill*> SpillMap;
  typedef std::map<unsigned int, SQSpill*>::const_iterator ConstIter;
  typedef std::map<unsigned int, SQSpill*>::iterator            Iter;

  virtual ~SQSpillMap() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SQSpillMap base class" << std::endl;
  }
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SQSpillMap* Clone() const {return NULL;}

  virtual bool   empty()                   const {return true;}
  virtual size_t  size()                   const {return 0;}
  virtual size_t count(unsigned int idkey) const {return 0;}
  virtual void   clear()                         {}

  virtual const SQSpill* get(unsigned int idkey) const {return NULL;}
  virtual       SQSpill* get(unsigned int idkey) {return NULL;}
  virtual       SQSpill* insert(const SQSpill *hit) {return NULL;}
  virtual       size_t   erase(unsigned int idkey) {return 0;}

  virtual ConstIter begin()                   const {return SpillMap().end();}
  virtual ConstIter  find(unsigned int idkey) const {return SpillMap().end();}
  virtual ConstIter   end()                   const {return SpillMap().end();}

  virtual Iter begin()                   {return SpillMap().end();}
  virtual Iter  find(unsigned int idkey) {return SpillMap().end();}
  virtual Iter   end()                   {return SpillMap().end();}

protected:
  SQSpillMap() {}

private:

  ClassDef(SQSpillMap, 1);
};



#endif /* _H_SQSpillMap_H_ */
