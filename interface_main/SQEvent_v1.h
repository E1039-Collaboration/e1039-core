/*
 * SQEvent_v1.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQEvent_v1_H_
#define _H_SQEvent_v1_H_

#include <phool/PHObject.h>

#include <map>
#include <iostream>
#include <limits>
#include <climits>

#include "SQEvent.h"

class SQEvent_v1: public SQEvent {

public:

	SQEvent_v1();

	virtual ~SQEvent_v1();

	void Reset();

	virtual void identify(std::ostream& os = std::cout) const;

	virtual int get_run_id() const {return _run_id;}
	virtual void set_run_id(const int a) {_run_id = a;}

	virtual int get_spill_id() const {return _spill_id;}
	virtual void set_spill_id(const int a) {_spill_id = a;}

	virtual int get_event_id() const {return _event_id;}
	virtual void set_event_id(const int a) {_event_id = a;}

	virtual int get_coda_event_id() const {return _coda_event_id;}
	virtual void set_coda_event_id(const int a) {_coda_event_id = a;}


	virtual int get_data_quality() const {return _data_quality;}
	virtual void set_data_quality(const int a) {_data_quality = a;}

	virtual int get_vme_time() const {return _vme_time;}
	virtual void set_vme_time(const int a) {_vme_time = a;}

	virtual bool get_trigger(const SQEvent::TriggerMask i) const {return (_trigger&(1<<i)) > 0 ;}
	virtual void set_trigger(const SQEvent::TriggerMask i, const bool a) {a ? (_trigger |= (1<<i)) : (_trigger &= ~(1<<i)) ;}

	virtual int get_raw_matrix(const unsigned short i) const {
		if(i<5) return _raw_matrix[i];
		return INT_MAX;
	}
	virtual void set_raw_matrix(const unsigned short i, const bool a) {
		if(i<5) _raw_matrix[i] = a;
		else std::cout<<"SQEvent_v1::set_raw_matrix: i>=5";
	}

	virtual int get_after_inh_matrix(const unsigned short i) const {
		if(i<5) return _after_inh_matrix[i];
		return INT_MAX;
	}
	virtual void set_after_inh_matrix(const unsigned short i, const bool a) {
		if(i<5) _after_inh_matrix[i] = a;
		else std::cout<<"SQEvent_v1::set_after_inh_matrix: i>=5";
	}

protected:
	int _run_id;
	int _spill_id;
	int _event_id;
	int _coda_event_id;

	unsigned short _trigger; //< NIM[1-5], MATRIX[1-5]

	int _raw_matrix[5];

	int _after_inh_matrix[5];

	int _data_quality;

	int _vme_time;

ClassDef(SQEvent_v1, 1);
};

#endif /* _H_SQEvent_v1_H_ */
