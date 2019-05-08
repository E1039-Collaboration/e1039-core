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

	virtual int  get_unix_time_begin() const {return std::numeric_limits<int>::max();}
	virtual void set_unix_time_begin(const int a) {}

	virtual int  get_unix_time_end() const {return std::numeric_limits<int>::max();}
	virtual void set_unix_time_end(const int a) {}

	virtual int  get_fpga_enabled(const int chan) const {return std::numeric_limits<int>::max();}
	virtual void set_fpga_enabled(const int chan, const int a) {}

	virtual int  get_nim_enabled(const int chan) const {return std::numeric_limits<int>::max();}
	virtual void set_nim_enabled(const int chan, const int a) {}

	virtual int  get_fpga_prescale(const int chan) const {return std::numeric_limits<int>::max();}
	virtual void set_fpga_prescale(const int chan, const int a) {}

	virtual int  get_nim_prescale(const int chan) const {return std::numeric_limits<int>::max();}
	virtual void set_nim_prescale(const int chan, const int a) {}

	virtual std::string get_run_desc() const {return "";}
	virtual void        set_run_desc(const std::string a) {}

	virtual int  get_n_fee_event() const {return std::numeric_limits<int>::max();}
	virtual void set_n_fee_event(const int a) {}

	virtual int  get_n_fee_prescale() const {return std::numeric_limits<int>::max();}
	virtual void set_n_fee_prescale(const int a) {}

	virtual int  get_n_run_desc() const {return std::numeric_limits<int>::max();}
	virtual void set_n_run_desc(const int a) {}

	virtual int  get_n_spill() const {return std::numeric_limits<int>::max();}
	virtual void set_n_spill(const int a) {}

	virtual int  get_n_evt_all() const {return std::numeric_limits<int>::max();}
	virtual void set_n_evt_all(const int a) {}

	virtual int  get_n_evt_dec() const {return std::numeric_limits<int>::max();}
	virtual void set_n_evt_dec(const int a) {}

	virtual int  get_n_phys_evt() const {return std::numeric_limits<int>::max();}
	virtual void set_n_phys_evt(const int a) {}

	virtual int  get_n_flush_evt() const {return std::numeric_limits<int>::max();}
	virtual void set_n_flush_evt(const int a) {}

	virtual int  get_n_hit() const {return std::numeric_limits<int>::max();}
	virtual void set_n_hit(const int a) {}

	virtual int  get_n_t_hit() const {return std::numeric_limits<int>::max();}
	virtual void set_n_t_hit(const int a) {}

	virtual int  get_n_hit_bad() const {return std::numeric_limits<int>::max();}
	virtual void set_n_hit_bad(const int a) {}

	virtual int  get_n_t_hit_bad() const {return std::numeric_limits<int>::max();}
	virtual void set_n_t_hit_bad(const int a) {}

	//virtual int  get_spill_count() const {return std::numeric_limits<int>::max();}
	//virtual void set_spill_count(const int a) {}

protected:

	SQRun(){};

ClassDef(SQRun, 1);
};

#endif /* _H_SQRun_H_ */
