/*
 * SEvent.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SEvent_H_
#define _H_SEvent_H_

#include "SHitMap.h"

#include <phool/PHObject.h>

#include <map>
#include <iostream>

class SEvent: public PHObject {

public:
	virtual ~SEvent() {}

	void Reset() {}

	virtual void identify(std::ostream& os = std::cout) const {
		std::cout
		<< "---SEvent::identify: abstract base-------------------"
		<< std::endl;
	}

	virtual void SetSHitMap(SHitMap* a) = 0;
	virtual SHitMap* GetSHitMap() = 0;

protected:

	SEvent(){};

ClassDef(SEvent, 1);
};

#endif /* _H_SEvent_H_ */
