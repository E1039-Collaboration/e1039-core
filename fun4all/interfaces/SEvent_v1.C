/*
 * SEvent_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */



#include "SEvent_v1.h"

#include "SHit.h"

using namespace std;

ClassImp(SEvent_v1)

SEvent_v1::SEvent_v1() :
_hitmap(nullptr)
{
}

SEvent_v1::~SEvent_v1() {
	Reset();
}

void SEvent_v1::Reset() {

	if(_hitmap) {
		delete _hitmap;
	}

	return;
}

void SEvent_v1::identify(std::ostream& os) const {
	  cout << "---SEvent_v1::identify:--------------------------" << endl;

	  if(_hitmap) {
		  cout << "\t ---Contains SHitMap:-------------------" << endl;
		  _hitmap->identify();
	  }

	  return;
}

void SEvent_v1::SetSHitMap(SHitMap* a) {
	if(_hitmap) delete _hitmap;
	_hitmap = a;
}
