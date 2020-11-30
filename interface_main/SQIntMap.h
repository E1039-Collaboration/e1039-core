/*
 * SQIntMap.h
 */
#ifndef _H_SQIntMap_H_
#define _H_SQIntMap_H_

#include <phool/PHObject.h>
#include <map>
#include <iostream>

/// A general-purpose SQ interface class that holds a list of PHObjects with key = integer.
/** The usage is nearly identical to SQSpillMap.
 */
class SQIntMap : public PHObject {
public:
  typedef std::map<unsigned int, PHObject*> ObjectMap;
  typedef ObjectMap::const_iterator ConstIter;
  typedef ObjectMap::iterator            Iter;

  virtual ~SQIntMap() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SQIntMap base class" << std::endl;
  }
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SQIntMap* Clone() const {return NULL;}

  virtual bool   empty()                   const {return true;}
  virtual size_t  size()                   const {return 0;}
  virtual size_t count(unsigned int idkey) const {return 0;}
  virtual void   clear()                         {}

  virtual const PHObject* get(unsigned int idkey) const {return NULL;}
  virtual       PHObject* get(unsigned int idkey) {return NULL;}
  virtual       PHObject* insert(const unsigned int idkey, const PHObject *item) {return NULL;}
  virtual       size_t   erase(unsigned int idkey) {return 0;}

  virtual ConstIter begin()                   const {return ObjectMap().end();}
  virtual ConstIter  find(unsigned int idkey) const {return ObjectMap().end();}
  virtual ConstIter   end()                   const {return ObjectMap().end();}

  virtual Iter begin()                   {return ObjectMap().end();}
  virtual Iter  find(unsigned int idkey) {return ObjectMap().end();}
  virtual Iter   end()                   {return ObjectMap().end();}

protected:
  SQIntMap() {}

private:
  ClassDef(SQIntMap, 1);
};

#endif /* _H_SQIntMap_H_ */
