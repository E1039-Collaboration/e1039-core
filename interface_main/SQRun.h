/*
 * SQRun.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQRun_H_
#define _H_SQRun_H_

#include <phool/PHObject.h>

#include <map>
#include <iostream>
#include <limits>

class SQRun: public PHObject {

public:

	virtual ~SQRun() {}

	void Reset() {}

	virtual void identify(std::ostream& os = std::cout) const {
		std::cout
		<< "---SQRun::identify: -------------------"
		<< std::endl;
	}

	virtual int get_run_id() const {return std::numeric_limits<int>::max();}
	virtual void set_run_id(const int a) {}

	virtual int get_spill_count() const {return std::numeric_limits<int>::max();}
	virtual void set_spill_count(const int a) {}


protected:

	SQRun(){};

ClassDef(SQRun, 1);
};

#endif /* _H_SQRun_H_ */
