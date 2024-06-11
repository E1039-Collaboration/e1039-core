/*
 * SQRun_v2.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQRun_v2_H_
#define _H_SQRun_v2_H_

#include <phool/PHObject.h>

#include <map>
#include <iostream>
#include <limits>
#include <climits>

#include "SQRun.h"

class SQRun_v2: public SQRun {

public:

	SQRun_v2();

	virtual ~SQRun_v2();

	void Reset();

	virtual void identify(std::ostream& os = std::cout) const;

	virtual int get_run_id() const {return _run_id;}
	virtual void set_run_id(const int a) {_run_id = a;}

	virtual int  get_unix_time_begin() const { return _utime_b; }
	virtual void set_unix_time_begin(const int a)   { _utime_b = a; }

	virtual int  get_unix_time_end() const { return _utime_e; }
	virtual void set_unix_time_end(const int a)   { _utime_e = a; }

	virtual int  get_fpga_enabled(const int chan) const { return _fpga_enabled[chan]; }
	virtual void set_fpga_enabled(const int chan, const int a) { _fpga_enabled[chan] = a; }

	virtual int  get_nim_enabled(const int chan) const { return _nim_enabled[chan]; }
	virtual void set_nim_enabled(const int chan, const int a) { _nim_enabled[chan] = a; }

	virtual int  get_fpga_prescale(const int chan) const { return _fpga_prescale[chan]; }
	virtual void set_fpga_prescale(const int chan, const int a) { _fpga_prescale[chan] = a; }

	virtual int  get_nim_prescale(const int chan) const { return _nim_prescale[chan]; }
	virtual void set_nim_prescale(const int chan, const int a) { _nim_prescale[chan] = a; }

        virtual int  get_v1495_id(const int chan) const  { return _v1495_id[chan]; }
        virtual void set_v1495_id(const int chan, const int id) { _v1495_id[chan] = id; }

	virtual std::string get_run_desc() const       { return _run_desc; }
	virtual void        set_run_desc(const std::string a) { _run_desc = a; }

	virtual int  get_n_fee_event() const { return _n_fee_event; }
	virtual void set_n_fee_event(const int a)   { _n_fee_event = a; }

	virtual int  get_n_fee_prescale() const { return _n_fee_prescale; }
	virtual void set_n_fee_prescale(const int a)   { _n_fee_prescale = a; }

	virtual int  get_n_run_desc() const { return _n_run_desc; }
	virtual void set_n_run_desc(const int a)   { _n_run_desc = a; }

	virtual int  get_n_spill() const { return _n_spill; }
	virtual void set_n_spill(const int a)   { _n_spill = a; }

	virtual int  get_n_evt_all() const { return _n_evt_all; }
	virtual void set_n_evt_all(const int a)   { _n_evt_all = a; }

	virtual int  get_n_evt_dec() const { return _n_evt_dec; }
	virtual void set_n_evt_dec(const int a)   { _n_evt_dec = a; }

	virtual int  get_n_phys_evt() const { return _n_phys_evt; }
	virtual void set_n_phys_evt(const int a)   { _n_phys_evt = a; }

	virtual int  get_n_phys_evt_bad() const { return _n_phys_evt_bad; }
	virtual void set_n_phys_evt_bad(const int a)   { _n_phys_evt_bad = a; }

	virtual int  get_n_flush_evt() const { return _n_flush_evt; }
	virtual void set_n_flush_evt(const int a)   { _n_flush_evt = a; }

	virtual int  get_n_flush_evt_bad() const { return _n_flush_evt_bad; }
	virtual void set_n_flush_evt_bad(const int a)   { _n_flush_evt_bad = a; }

	virtual int  get_n_hit() const { return _n_hit; }
	virtual void set_n_hit(const int a)   { _n_hit = a; }

	virtual int  get_n_t_hit() const { return _n_t_hit; }
	virtual void set_n_t_hit(const int a)   { _n_t_hit = a; }

	virtual int  get_n_hit_bad() const { return _n_hit_bad; }
	virtual void set_n_hit_bad(const int a)   { _n_hit_bad = a; }

	virtual int  get_n_t_hit_bad() const { return _n_t_hit_bad; }
	virtual void set_n_t_hit_bad(const int a)   { _n_t_hit_bad = a; }

	virtual int  get_n_v1495() const { return _n_v1495; }
	virtual void set_n_v1495(const int a)   { _n_v1495 = a; }

	virtual int  get_n_v1495_d1ad() const { return _n_v1495_d1ad; }
	virtual void set_n_v1495_d1ad(const int a)   { _n_v1495_d1ad = a; }

	virtual int  get_n_v1495_d2ad() const { return _n_v1495_d2ad; }
	virtual void set_n_v1495_d2ad(const int a)   { _n_v1495_d2ad = a; }

	virtual int  get_n_v1495_d3ad() const { return _n_v1495_d3ad; }
	virtual void set_n_v1495_d3ad(const int a)   { _n_v1495_d3ad = a; }

protected:
	int _run_id;

  int _utime_b; //< Unix time of run beginning
  int _utime_e; //< Unix time of run end

  int _fpga_enabled [5];
  int  _nim_enabled [5];
  int _fpga_prescale[5];
  int  _nim_prescale[3];
  int _v1495_id[5];

  std::string _run_desc;
  
  int _n_fee_event;
  int _n_fee_prescale;
  int _n_run_desc;
  int _n_spill;
  int _n_evt_all;   //< N of all real (i.e. triggered) events
  int _n_evt_dec;   //< N of decoded real events
  int _n_phys_evt;  //< N of Coda standard-physics events
  int _n_phys_evt_bad;
  int _n_flush_evt; //< N of Coda flush events
  int _n_flush_evt_bad;
  int _n_hit;       //< N of Taiwan-TDC hits
  int _n_t_hit;     //< N of v1495-TDC hits
  int _n_hit_bad;   //< N of bad hits
  int _n_t_hit_bad; //< N of bad t-hits.  Not implemented
  int _n_v1495;     //< N of all v1495 events
  int _n_v1495_d1ad;//< N of d1ad v1495 events
  int _n_v1495_d2ad;//< N of d2ad v1495 events
  int _n_v1495_d3ad;//< N of d3ad v1495 events

ClassDef(SQRun_v2, 1);
};

#endif /* _H_SQRun_v2_H_ */
