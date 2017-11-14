/*
 * SEvent_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SEvent_v1_H_
#define _H_SEvent_v1_H_

#include "SEvent.h"

#include <phool/PHObject.h>

#include <map>
#include <iostream>

class SEvent_v1: public SEvent {

public:

	SEvent_v1();

	virtual ~SEvent_v1();

	void Reset();

	virtual void identify(std::ostream& os = std::cout) const;

	virtual void SetSHitMap(SHitMap* a);
	virtual SHitMap* GetSHitMap() { return _hitmap;}

protected:
	SHitMap* _hitmap;

ClassDef(SEvent_v1, 1);
};

#endif /* _H_SEvent_v1_H_ */
