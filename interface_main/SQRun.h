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

/// An SQ interface class to hold the run-level info.
class SQRun: public PHObject {

public:

	virtual ~SQRun() {}

	void Reset() {}

	virtual void identify(std::ostream& os = std::cout) const {
		std::cout
		<< "---SQRun::identify: -------------------"
		<< std::endl;
	}

	virtual int get_run_id() const {return std::numeric_limits<int>::max();} ///< Return the run ID.
	virtual void set_run_id(const int a) {}

	virtual int  get_unix_time_begin() const {return std::numeric_limits<int>::max();} ///< Return the Unix time when this run began.
	virtual void set_unix_time_begin(const int a) {}

	virtual int  get_unix_time_end() const {return std::numeric_limits<int>::max();} ///< Return the Unix time when this run ended.
	virtual void set_unix_time_end(const int a) {}

	virtual int  get_fpga_enabled(const int chan) const {return std::numeric_limits<int>::max();} ///< Return 'true' if the given channel ('chan') of the FPGA trigger was enabled in this run.
	virtual void set_fpga_enabled(const int chan, const int a) {}

	virtual int  get_nim_enabled(const int chan) const {return std::numeric_limits<int>::max();} ///< Return 'true' if the given channel ('chan') of the NIM trigger was enabled in this run.
	virtual void set_nim_enabled(const int chan, const int a) {}

	virtual int  get_fpga_prescale(const int chan) const {return std::numeric_limits<int>::max();} ///< Return the prescale factor of the given channel ('chan') of the FPGA trigger in this run.
	virtual void set_fpga_prescale(const int chan, const int a) {}

	virtual int  get_nim_prescale(const int chan) const {return std::numeric_limits<int>::max();} ///< Return the prescale factor of the given channel ('chan') of the NIM trigger in this run.
	virtual void set_nim_prescale(const int chan, const int a) {}

	virtual std::string get_run_desc() const {return "";} ///< Return the run description of this run.
	virtual void        set_run_desc(const std::string a) {}

	virtual int  get_n_fee_event() const {return std::numeric_limits<int>::max();} ///< Return the number of FEE events.
	virtual void set_n_fee_event(const int a) {}

	virtual int  get_n_fee_prescale() const {return std::numeric_limits<int>::max();} ///< Return the number of FEE-prescale events.
	virtual void set_n_fee_prescale(const int a) {}

	virtual int  get_n_run_desc() const {return std::numeric_limits<int>::max();} ///< Return the number of run-description events.
	virtual void set_n_run_desc(const int a) {}

	virtual int  get_n_spill() const {return std::numeric_limits<int>::max();} ///< Return the number of spill events.
	virtual void set_n_spill(const int a) {}

	virtual int  get_n_evt_all() const {return std::numeric_limits<int>::max();} ///< Return the number of all recorded events.
	virtual void set_n_evt_all(const int a) {}

	virtual int  get_n_evt_dec() const {return std::numeric_limits<int>::max();} ///< Return the number of decoded events.  The online decoding usually skips a part of events in order to finish in time.
	virtual void set_n_evt_dec(const int a) {}

	virtual int  get_n_phys_evt() const {return std::numeric_limits<int>::max();} ///< Return the number of all PHYSICS events.
	virtual void set_n_phys_evt(const int a) {}

	virtual int  get_n_phys_evt_bad() const {return std::numeric_limits<int>::max();} ///< Return the number of bad PHYSICS events.
	virtual void set_n_phys_evt_bad(const int a) {}

	virtual int  get_n_flush_evt() const {return std::numeric_limits<int>::max();} ///< Return the number of all FLUSH events.
	virtual void set_n_flush_evt(const int a) {}

	virtual int  get_n_flush_evt_bad() const {return std::numeric_limits<int>::max();} ///< Return the number of bad FLUSH events.
	virtual void set_n_flush_evt_bad(const int a) {}

	virtual int  get_n_hit() const {return std::numeric_limits<int>::max();} ///< Return the number of all Taiwan-TDC hits.
	virtual void set_n_hit(const int a) {}

	virtual int  get_n_t_hit() const {return std::numeric_limits<int>::max();} ///< Return the number of all V1495-TDC (i.e. trigger) hits.
	virtual void set_n_t_hit(const int a) {}

	virtual int  get_n_hit_bad() const {return std::numeric_limits<int>::max();} ///< Return the number of bad Taiwan-TDC hits.
	virtual void set_n_hit_bad(const int a) {}

	virtual int  get_n_t_hit_bad() const {return std::numeric_limits<int>::max();} ///< Return the number of bad V1495-TDC (i.e. trigger) hits.
	virtual void set_n_t_hit_bad(const int a) {}

	virtual int  get_n_v1495() const {return std::numeric_limits<int>::max();} ///< Return the number of all V1495 events.
	virtual void set_n_v1495(const int a) {}

	virtual int  get_n_v1495_d1ad() const {return std::numeric_limits<int>::max();} ///< Return the number of V1495 events having 'd1ad'.
	virtual void set_n_v1495_d1ad(const int a) {}

	virtual int  get_n_v1495_d2ad() const {return std::numeric_limits<int>::max();} ///< Return the number of V1495 events having 'd2ad'.
	virtual void set_n_v1495_d2ad(const int a) {}

	virtual int  get_n_v1495_d3ad() const {return std::numeric_limits<int>::max();} ///< Return the number of V1495 events having 'd3ad'.
	virtual void set_n_v1495_d3ad(const int a) {}

protected:

	SQRun(){};

ClassDef(SQRun, 1);
};

#endif /* _H_SQRun_H_ */
