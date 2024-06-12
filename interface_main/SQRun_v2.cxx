/*
 * SQRun_v2.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */



#include "SQRun_v2.h"

using namespace std;

ClassImp(SQRun_v2)

SQRun_v2::SQRun_v2() :
  _run_id(INT_MAX),
  _utime_b(INT_MAX),
  _utime_e(INT_MAX),
  _run_desc(""),
  _n_fee_event(INT_MAX),
  _n_fee_prescale(INT_MAX),
  _n_run_desc(INT_MAX),
  _n_spill(INT_MAX),
  _n_evt_all(INT_MAX),
  _n_evt_dec(INT_MAX),
  _n_phys_evt(INT_MAX),
  _n_phys_evt_bad(INT_MAX),
  _n_flush_evt(INT_MAX),
  _n_flush_evt_bad(INT_MAX),
  _n_hit(INT_MAX),
  _n_t_hit(INT_MAX),
  _n_hit_bad(INT_MAX),
  _n_t_hit_bad(INT_MAX), 
  _n_v1495(INT_MAX), 
  _n_v1495_d1ad(INT_MAX),
  _n_v1495_d2ad(INT_MAX),
  _n_v1495_d3ad(INT_MAX)

{
  memset(_fpga_enabled , 0, sizeof(_fpga_enabled ));
  memset( _nim_enabled , 0, sizeof( _nim_enabled ));
  memset(_fpga_prescale, 0, sizeof(_fpga_prescale));
  memset( _nim_prescale, 0, sizeof( _nim_prescale));
  memset(     _v1495_id, 0, sizeof(_v1495_id));
}

SQRun_v2::~SQRun_v2() {
	Reset();
}

void SQRun_v2::Reset() {
	return;
}

void SQRun_v2::identify(std::ostream& os) const {
	  cout << "---SQRun_v2::identify:--------------------------" << endl;
	  cout
	  << "runID: " << _run_id
            //<< "spillID: " << _spill_count
	  <<endl;
	  cout <<endl;
	  return;
}
