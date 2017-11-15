/*
 * SQSpillMap_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQSpillMap_v1_H_
#define _H_SQSpillMap_v1_H_

#include "SQSpillMap.h"
#include "SQSpill.h"

#include <phool/PHObject.h>
#include <map>
#include <iostream>

class SQSpillMap_v1 : public SQSpillMap {

public:

  SQSpillMap_v1();
  SQSpillMap_v1(const SQSpillMap_v1& hitmap);
  SQSpillMap_v1& operator=(const SQSpillMap_v1& hitmap);
  virtual ~SQSpillMap_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SQSpillMap* Clone() const {return new SQSpillMap_v1(*this);}

  bool   empty()                   const {return _map.empty();}
  size_t  size()                   const {return _map.size();}
  size_t count(unsigned int idkey) const {return _map.count(idkey);}
  void   clear()                         {Reset();}

  const SQSpill* get(unsigned int idkey) const;
        SQSpill* get(unsigned int idkey);
        SQSpill* insert(const SQSpill *hit);
        size_t   erase(unsigned int idkey) {
	  delete _map[idkey]; return _map.erase(idkey);
	}

  ConstIter begin()                   const {return _map.begin();}
  ConstIter  find(unsigned int idkey) const {return _map.find(idkey);}
  ConstIter   end()                   const {return _map.end();}

  Iter begin()                   {return _map.begin();}
  Iter  find(unsigned int idkey) {return _map.find(idkey);}
  Iter   end()                   {return _map.end();}

private:
  SpillMap _map;

  ClassDef(SQSpillMap_v1, 1);
};

#endif
