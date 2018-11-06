/*
 * SQIntMap_v1.h
 */
#ifndef _H_SQIntMap_v1_H_
#define _H_SQIntMap_v1_H_
#include "SQIntMap.h"

class SQIntMap_v1 : public SQIntMap {
public:
  SQIntMap_v1();
  SQIntMap_v1(const SQIntMap_v1& map);
  SQIntMap_v1& operator=(const SQIntMap_v1& map);
  virtual ~SQIntMap_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SQIntMap* Clone() const {return new SQIntMap_v1(*this);}

  bool   empty()                   const {return _map.empty();}
  size_t  size()                   const {return _map.size();}
  size_t count(unsigned int idkey) const {return _map.count(idkey);}
  void   clear()                         {Reset();}

  const PHObject* get(unsigned int idkey) const;
        PHObject* get(unsigned int idkey);
        PHObject* insert(unsigned int idkey, const PHObject *item);
        size_t erase(unsigned int idkey) {
	  delete _map[idkey];
          return _map.erase(idkey);
	}

  ConstIter begin()                   const {return _map.begin();}
  ConstIter  find(unsigned int idkey) const {return _map.find(idkey);}
  ConstIter   end()                   const {return _map.end();}

  Iter begin()                   {return _map.begin();}
  Iter  find(unsigned int idkey) {return _map.find(idkey);}
  Iter   end()                   {return _map.end();}

private:
  ObjectMap _map;

  ClassDef(SQIntMap_v1, 1);
};

#endif
