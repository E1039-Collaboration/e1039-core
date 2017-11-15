/*
 * SQRun_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQRun_v1_H_
#define _H_SQRun_v1_H_

#include "SQRun.h"

#include <phool/PHObject.h>

#include <map>
#include <iostream>
#include <limits>
#include <climits>

class SQRun_v1: public SQRun {

public:

	SQRun_v1();

	virtual ~SQRun_v1();

	void Reset();

	virtual void identify(std::ostream& os = std::cout) const;

	virtual int get_run_id() const {return _run_id;}
	virtual void set_run_id(const int a) {_run_id = a;}

	virtual int get_spill_count() const {return _spill_count;}
	virtual void set_spill_count(const int a) {_spill_count = a;}

protected:
	int _run_id;
	int _spill_count;

ClassDef(SQRun_v1, 1);
};

#endif /* _H_SQRun_v1_H_ */
