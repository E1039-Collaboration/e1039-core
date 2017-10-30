/*
 * SHitMap_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SHitMap_v1_H_
#define _H_SHitMap_v1_H_

#include "SHitMap.h"
#include "SHit.h"

#include <phool/PHObject.h>
#include <map>
#include <iostream>

class SHitMap_v1 : public SHitMap {

public:

  SHitMap_v1();
  SHitMap_v1(const SHitMap_v1& hitmap);
  SHitMap_v1& operator=(const SHitMap_v1& hitmap);
  virtual ~SHitMap_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SHitMap* Clone() const {return new SHitMap_v1(*this);}

  bool   empty()                   const {return _map.empty();}
  size_t  size()                   const {return _map.size();}
  size_t count(unsigned int idkey) const {return _map.count(idkey);}
  void   clear()                         {Reset();}

  const SHit* get(unsigned int idkey) const;
        SHit* get(unsigned int idkey);
        SHit* insert(const SHit *hit);
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
  HitMap _map;

  ClassDef(SHitMap_v1, 1);
};

#endif
