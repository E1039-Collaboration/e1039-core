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
  int  isValid() const {return 1;}
  SQEvent* Clone() const {return new SQEvent_v1(*this);}

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

	virtual unsigned short get_trigger() const {return _trigger;}
	virtual void           set_trigger(const unsigned short a) {_trigger = a;}

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

        virtual int  get_qie_presum(const unsigned short i) const {
          if (i<4) return _qie_presums[i]; 
          return INT_MAX;
        }
	virtual void set_qie_presum(const unsigned short i, const int a) {
          if(i<4) _qie_presums[i] = a;
          else std::cout<<"SQEvent_v1::set_qie_presum: i>=4";
        }

        virtual int  get_qie_trigger_count() const { return _qie_trig_cnt; }
	virtual void set_qie_trigger_count(const int a)   { _qie_trig_cnt = a; }

        virtual int  get_qie_turn_id() const { return _qie_turn_id; }
	virtual void set_qie_turn_id(const int a)   { _qie_turn_id = a; }

        virtual int  get_qie_rf_id() const { return _qie_rf_id; }
	virtual void set_qie_rf_id(const int a)   { _qie_rf_id = a; }

        virtual int  get_qie_rf_intensity(const short i) const {
          if (abs(i)<=16) return _qie_rf_inte[i+16]; 
          return INT_MAX;
        }
	virtual void set_qie_rf_intensity(const short i, const int a) {
          if(abs(i)<=16) _qie_rf_inte[i+16] = a;
          else std::cout<<"SQEvent_v1::set_qie_rf_intensity: abs(i)>16";
        }

        virtual short get_flag_v1495() const { return _flag_v1495; }
	virtual void  set_flag_v1495(const short a) { _flag_v1495 = a; }

        virtual short get_n_board_qie() const { return _n_board_qie; }
	virtual void  set_n_board_qie(const short a) { _n_board_qie = a; }

        virtual short get_n_board_v1495() const { return _n_board_v1495; }
	virtual void  set_n_board_v1495(const short a) { _n_board_v1495 = a; }

        virtual short get_n_board_taiwan() const { return _n_board_taiwan; }
	virtual void  set_n_board_taiwan(const short a) { _n_board_taiwan = a; }

        virtual short get_n_board_trig_bit() const { return _n_board_trig_b; }
	virtual void  set_n_board_trig_bit(const short a) { _n_board_trig_b = a; }

        virtual short get_n_board_trig_count() const { return _n_board_trig_c; }
	virtual void  set_n_board_trig_count(const short a) { _n_board_trig_c = a; }



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

        int _qie_presums[4];
        int _qie_trig_cnt;
        int _qie_turn_id;
        int _qie_rf_id;
        int _qie_rf_inte[33];

        short _flag_v1495;
        short _n_board_qie;
        short _n_board_v1495;
        short _n_board_taiwan;
        short _n_board_trig_b;
        short _n_board_trig_c;

ClassDef(SQEvent_v1, 1);
};

#endif /* _H_SQEvent_v1_H_ */
