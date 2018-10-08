/*
 * SQStringMap_v1.h
 */
#ifndef _H_SQStringMap_v1_H_
#define _H_SQStringMap_v1_H_
#include "SQStringMap.h"

class SQStringMap_v1 : public SQStringMap {
public:
  SQStringMap_v1();
  SQStringMap_v1(const SQStringMap_v1& map);
  SQStringMap_v1& operator=(const SQStringMap_v1& map);
  virtual ~SQStringMap_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SQStringMap* Clone() const {return new SQStringMap_v1(*this);}

  bool   empty()                const {return _map.empty();}
  size_t  size()                const {return _map.size();}
  size_t count(std::string key) const {return _map.count(key);}
  void   clear()                      {Reset();}

  const PHObject* get(std::string key) const;
        PHObject* get(std::string key);
        PHObject* insert(std::string key, const PHObject *item);
        size_t erase(std::string key) {
	  delete _map[key];
          return _map.erase(key);
	}

  ConstIter begin()                const {return _map.begin();}
  ConstIter  find(std::string key) const {return _map.find(key);}
  ConstIter   end()                const {return _map.end();}

  Iter begin()                {return _map.begin();}
  Iter  find(std::string key) {return _map.find(key);}
  Iter   end()                {return _map.end();}

private:
  ObjectMap _map;

  ClassDef(SQStringMap_v1, 1);
};

#endif
