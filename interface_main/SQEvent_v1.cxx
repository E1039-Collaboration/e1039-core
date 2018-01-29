/*
 * SQEvent_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */



#include "SQEvent_v1.h"

using namespace std;

ClassImp(SQEvent_v1)

SQEvent_v1::SQEvent_v1() :
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

SQEvent_v1::~SQEvent_v1() {
	Reset();
}

void SQEvent_v1::Reset() {
	return;
}

void SQEvent_v1::identify(std::ostream& os) const {
	  cout << "---SQEvent_v1::identify:--------------------------" << endl;
	  cout
	  << "runID: " << _run_id
	  << " spillID: " << _spill_id
	  << " eventID: " << _event_id
	  << " codaEventID: " << _coda_event_id
	  << endl;
	  for(int i = SQEvent::NIM1; i<=SQEvent::NIM5; ++i) {
	  	int name = i - SQEvent::NIM1 + 1;
		  cout <<"NIM"<<name<<": " << get_trigger(static_cast<SQEvent::TriggerMask>(i)) << "; ";
	  }
	  cout <<endl;
	  for(int i = SQEvent::MATRIX1; i<=SQEvent::MATRIX5; ++i){
	  	int name = i - SQEvent::MATRIX1 + 1;
		  cout <<"MATRIX"<<name<<": " << get_trigger(static_cast<SQEvent::TriggerMask>(i)) << "; ";
	  }
	  cout <<endl;
	  return;
}
