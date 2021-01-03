/*
 * SQStringMap.h
 */
#ifndef _H_SQStringMap_H_
#define _H_SQStringMap_H_

#include <phool/PHObject.h>
#include <map>
#include <iostream>

/// A general-purpose SQ interface class that holds a list of PHObjects with key = string.
/** The usage is nearly identical to SQSpillMap.
 */
class SQStringMap : public PHObject {
public:
  typedef std::map<std::string, PHObject*> ObjectMap;
  typedef ObjectMap::const_iterator ConstIter;
  typedef ObjectMap::iterator            Iter;

  virtual ~SQStringMap() {}

  virtual void identify(std::ostream& os = std::cout) const {
    os << "SQStringMap base class" << std::endl;
  }
  virtual void Reset() {}
  virtual int  isValid() const {return 0;}
  virtual SQStringMap* Clone() const {return NULL;}

  virtual bool   empty()                const {return true;}
  virtual size_t  size()                const {return 0;}
  virtual size_t count(std::string key) const {return 0;}
  virtual void   clear()                         {}

  virtual const PHObject* get(std::string key) const {return NULL;}
  virtual       PHObject* get(std::string key) {return NULL;}
  virtual       PHObject* insert(const std::string key, const PHObject *item) {return NULL;}
  virtual       size_t   erase(std::string key) {return 0;}

  virtual ConstIter begin()                const {return ObjectMap().end();}
  virtual ConstIter  find(std::string key) const {return ObjectMap().end();}
  virtual ConstIter   end()                const {return ObjectMap().end();}

  virtual Iter begin()                {return ObjectMap().end();}
  virtual Iter  find(std::string key) {return ObjectMap().end();}
  virtual Iter   end()                {return ObjectMap().end();}

protected:
  SQStringMap() {}

private:
  ClassDef(SQStringMap, 1);
};

#endif /* _H_SQStringMap_H_ */
