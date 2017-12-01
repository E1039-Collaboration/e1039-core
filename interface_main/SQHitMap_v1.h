/*
 * SQHitMap_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_SQHitMap_v1_H_
#define _H_SQHitMap_v1_H_

#include <phool/PHObject.h>
#include <map>
#include <iostream>

#include "SQHit.h"
#include "SQHitMap.h"

class SQHitMap_v1 : public SQHitMap {

public:

  SQHitMap_v1();
  SQHitMap_v1(const SQHitMap_v1& hitmap);
  SQHitMap_v1& operator=(const SQHitMap_v1& hitmap);
  virtual ~SQHitMap_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int  isValid() const {return 1;}
  SQHitMap* Clone() const {return new SQHitMap_v1(*this);}

  bool   empty()                   const {return _map.empty();}
  size_t  size()                   const {return _map.size();}
  size_t count(unsigned int idkey) const {return _map.count(idkey);}
  void   clear()                         {Reset();}

  const SQHit* get(unsigned int idkey) const;
        SQHit* get(unsigned int idkey);
        SQHit* insert(const SQHit *hit);
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

  ClassDef(SQHitMap_v1, 1);
};

#endif
