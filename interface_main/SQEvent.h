/*
 * SQEvent.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQEvent_H_
#define _H_SQEvent_H_

#include <phool/PHObject.h>

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

	virtual int get_run_id() const = 0; ///< Return the run ID.
	virtual void set_run_id(const int a) = 0;

	virtual int get_spill_id() const = 0; ///< Return the spill ID.
	virtual void set_spill_id(const int a) = 0;

	virtual int get_event_id() const = 0; ///< Return the event ID, which is unique per run.
	virtual void set_event_id(const int a) = 0;

	virtual int get_coda_event_id() const = 0; ///< Return the Coda-event ID, which is unique per run.
	virtual void set_coda_event_id(const int a) = 0;


	virtual int get_data_quality() const = 0; ///< Return the data-quality bits.
	virtual void set_data_quality(const int a) = 0;

	virtual int get_vme_time() const = 0; ///< Return the VME time.
	virtual void set_vme_time(const int a) = 0;

	virtual bool get_trigger(const SQEvent::TriggerMask i) const = 0; ///< Return the trigger bit (fired or not) of the selected trigger channel.
	virtual void set_trigger(const SQEvent::TriggerMask i, const bool a) = 0;

	virtual unsigned short get_trigger() const = 0; ///< Return the full trigger bits.
	virtual void           set_trigger(const unsigned short a) = 0;

	virtual int get_raw_matrix(const unsigned short i) const = 0; ///< [Not Useful] Return the raw count of the selected trigger channel.
	virtual void set_raw_matrix(const unsigned short i, const bool a) = 0;

	virtual int get_after_inh_matrix(const unsigned short i) const = 0; ///< [Not Useful] Return the after-inhibited count of the selected trigger channel.
	virtual void set_after_inh_matrix(const unsigned short i, const bool a) = 0;


        virtual int  get_qie_presum(const unsigned short i) const = 0; ///< Return the i-th QIE presum, where i=0...3.
	virtual void set_qie_presum(const unsigned short i, const int a) = 0;

        virtual int  get_qie_trigger_count() const = 0; ///< Return the QIE trigger counts.
	virtual void set_qie_trigger_count(const int a) = 0;

        virtual int  get_qie_turn_id() const = 0; ///< Return the QIE turn ID.
	virtual void set_qie_turn_id(const int a) = 0;

        virtual int  get_qie_rf_id() const = 0; ///< Return the QIE RF ID.
	virtual void set_qie_rf_id(const int a) = 0;

        virtual int  get_qie_rf_intensity(const short i) const = 0; ///< Return the i-th QIE RF intensity, where i=-16...+16.
	virtual void set_qie_rf_intensity(const short i, const int a) = 0;


        virtual short get_flag_v1495() const = 0; ///< Return the quality flag of the V1495 readout.
	virtual void  set_flag_v1495(const short a) = 0;

        virtual short get_n_board_qie() const = 0; ///< Return the number of QIE boards read out.
	virtual void  set_n_board_qie(const short a) = 0;

        virtual short get_n_board_v1495() const = 0; ///< Return the number of V1495 boards read out.
	virtual void  set_n_board_v1495(const short a) = 0;

        virtual short get_n_board_taiwan() const = 0; ///< Return the number of Taiwan-TDC boards read out.
	virtual void  set_n_board_taiwan(const short a) = 0;

        virtual short get_n_board_trig_bit() const = 0; ///< Return the number of trigger-bit boards read out.
	virtual void  set_n_board_trig_bit(const short a) = 0;

        virtual short get_n_board_trig_count() const = 0; ///< Return the number of trigger-count boards read out.
	virtual void  set_n_board_trig_count(const short a) = 0;


protected:

	SQEvent(){};

ClassDef(SQEvent, 1);
};

#endif /* _H_SQEvent_H_ */
