/*
 * SEvent_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */



#include "SEvent_v1.h"

#include "SHit.h"

using namespace std;

ClassImp(SEvent_v1)

SEvent_v1::SEvent_v1() :
_run_id(INT_MAX),
_spill_id(INT_MAX),
_event_id(INT_MAX),
_coda_event_id(INT_MAX),
_trigger(0),
_data_quality(INT_MAX),
_vme_time(INT_MAX)
{
	for(int i=0;i<5;++i) _raw_matrix[i] = 0;
	for(int i=0;i<5;++i) _after_inh_matrix[i] = 0;
}

SEvent_v1::~SEvent_v1() {
	Reset();
}

void SEvent_v1::Reset() {
	return;
}

void SEvent_v1::identify(std::ostream& os) const {
	  cout << "---SEvent_v1::identify:--------------------------" << endl;
	  cout
	  << "runID: " << _run_id
	  << "spillID: " << _spill_id
	  << "eventID: " << _event_id
	  << "codaEventID: " << _coda_event_id
	  << endl;
	  for(int i = SEvent::NIM1; i<=SEvent::NIM5; ++i) {
		  cout <<"NIM"<<i<<": " << get_trigger(static_cast<SEvent::TriggerMask>(i)) << "; ";
	  }
	  cout <<endl;
	  for(int i = SEvent::MATRIX1; i<=SEvent::MATRIX5; ++i){
		  cout <<"MATRIX"<<i<<": " << get_trigger(static_cast<SEvent::TriggerMask>(i)) << "; ";
	  }
	  cout <<endl;
	  return;
}
