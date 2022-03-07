/*
 * SQEvent.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQEvent_H_
#define _H_SQEvent_H_

#include <phool/PHObject.h>
#include <climits>
#include <map>
#include <iostream>

/// An SQ interface class to hold one event header.
class SQEvent: public PHObject {

public:

	enum TriggerMask {
		NIM1 = 0,
		NIM2 = 1,
		NIM3 = 2,
		NIM4 = 3,
		NIM5 = 4,
		MATRIX1 = 5,
		MATRIX2 = 6,
		MATRIX3 = 7,
		MATRIX4 = 8,
		MATRIX5 = 9
	};

	virtual ~SQEvent() {}

	void Reset() {}

	virtual void identify(std::ostream& os = std::cout) const {
		std::cout
		<< "---SQEvent::identify: abstract base-------------------"
		<< std::endl;
	}

        /// Return the run ID.
	virtual int get_run_id() const = 0;
	virtual void set_run_id(const int a) = 0;

        /// Return the spill ID.
	virtual int get_spill_id() const = 0;
	virtual void set_spill_id(const int a) = 0;

        /// Return the event ID, which is unique per run.
	virtual int get_event_id() const = 0;
	virtual void set_event_id(const int a) = 0;

        /// [Obsolete] Use `SQHardEvent` instead.
	virtual int get_coda_event_id() const {return INT_MAX;}
	virtual void set_coda_event_id(const int a) {;}

        /// Return the data-quality bits.
	virtual int get_data_quality() const = 0;
	virtual void set_data_quality(const int a) = 0;

        /// [Obsolete] Use `SQHardEvent` instead.
	virtual int get_vme_time() const {return INT_MAX;}
	virtual void set_vme_time(const int a) {;}

        /// Return the trigger bit (fired or not) of the selected trigger channel.
	virtual bool get_trigger(const SQEvent::TriggerMask i) const = 0; 
	virtual void set_trigger(const SQEvent::TriggerMask i, const bool a) = 0;

        /// Return the full trigger bits.
	virtual unsigned short get_trigger() const = 0;
	virtual void           set_trigger(const unsigned short a) = 0;

        /// [Obsolete] Use `SQHardEvent` instead.
	virtual int get_raw_matrix(const unsigned short i) const { return INT_MAX; }
	virtual void set_raw_matrix(const unsigned short i, const bool a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
	virtual int get_after_inh_matrix(const unsigned short i) const { return INT_MAX; }
	virtual void set_after_inh_matrix(const unsigned short i, const bool a) {;}


        /// Return the i-th QIE presum, where i=0...3.
        virtual int  get_qie_presum(const unsigned short i) const = 0; 
	virtual void set_qie_presum(const unsigned short i, const int a) = 0;

        /// Return the QIE trigger counts.
        virtual int  get_qie_trigger_count() const = 0;
	virtual void set_qie_trigger_count(const int a) = 0;

        /// Return the QIE turn ID.
        virtual int  get_qie_turn_id() const = 0; 
	virtual void set_qie_turn_id(const int a) = 0;

        /// Return the QIE RF ID.
        virtual int  get_qie_rf_id() const = 0; 
	virtual void set_qie_rf_id(const int a) = 0;

        /// Return the i-th QIE RF intensity, where i=-16...+16.
        virtual int  get_qie_rf_intensity(const short i) const = 0; 
	virtual void set_qie_rf_intensity(const short i, const int a) = 0;


        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_flag_v1495() const { return SHRT_MAX; }
	virtual void  set_flag_v1495(const short a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_n_board_qie() const { return SHRT_MAX; }
	virtual void  set_n_board_qie(const short a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_n_board_v1495() const { return SHRT_MAX; }
	virtual void  set_n_board_v1495(const short a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_n_board_taiwan() const { return SHRT_MAX; }
	virtual void  set_n_board_taiwan(const short a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_n_board_trig_bit() const { return SHRT_MAX; }
	virtual void  set_n_board_trig_bit(const short a) {;}

        /// [Obsolete] Use `SQHardEvent` instead.
        virtual short get_n_board_trig_count() const { return SHRT_MAX; }
	virtual void  set_n_board_trig_count(const short a) {;}


protected:

	SQEvent(){};

ClassDef(SQEvent, 1);
};

#endif /* _H_SQEvent_H_ */
