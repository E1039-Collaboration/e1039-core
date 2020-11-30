/*
 * SpillMap.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQSpillMap_H_
#define _H_SQSpillMap_H_


#include <phool/PHObject.h>
#include <map>
#include <iostream>

#include "SQSpill.h"

/// An SQ interface class to hold a list of SQSpill objects.
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

  virtual bool   empty()                   const {return true;} ///< Return 'true' if this object holds no spill.
  virtual size_t  size()                   const {return 0;} ///< Return the number of spills held.
  virtual size_t count(unsigned int idkey) const {return 0;} ///< Return the number of spills having spill ID = 'idkey'.  Must be '0' or '1'.
  virtual void   clear()                         {} ///< Clear the list.

  virtual const SQSpill* get(unsigned int idkey) const {return NULL;} ///< Return the SQSpill entry having spill ID = 'idkey'.  Return '0' if no entry exists.
  virtual       SQSpill* get(unsigned int idkey) {return NULL;} ///< Return the SQSpill entry having spill ID = 'idkey'.  Return '0' if no entry exists.
  virtual       SQSpill* insert(const SQSpill *hit) {return NULL;} ///< Insert the given SQSpill object.
  virtual       size_t   erase(unsigned int idkey) {return 0;} ///< Erase the SQSpill entry having spill ID = 'idkey'.

  virtual ConstIter begin()                   const {return SpillMap().end();} ///< Return the const begin iterator.
  virtual ConstIter  find(unsigned int idkey) const {return SpillMap().end();} ///< Return the const iterator of the SQSpill entry having spill ID = 'idkey'.
  virtual ConstIter   end()                   const {return SpillMap().end();} ///< Return the const end iterator.

  virtual Iter begin()                   {return SpillMap().end();} ///< Return the begin iterator.
  virtual Iter  find(unsigned int idkey) {return SpillMap().end();} ///< Return the iterator of the SQSpill entry having spill ID = 'idkey'.
  virtual Iter   end()                   {return SpillMap().end();} ///< Return the end iterator.

protected:
  SQSpillMap() {}

private:

  ClassDef(SQSpillMap, 1);
};



#endif /* _H_SQSpillMap_H_ */
