/*
 * SQRun_v1.C
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */



#include "SQRun_v1.h"

using namespace std;

ClassImp(SQRun_v1)

SQRun_v1::SQRun_v1() :
_run_id(INT_MAX),
_spill_count(INT_MAX)
{
}

SQRun_v1::~SQRun_v1() {
	Reset();
}

void SQRun_v1::Reset() {
	return;
}

void SQRun_v1::identify(std::ostream& os) const {
	  cout << "---SQRun_v1::identify:--------------------------" << endl;
	  cout
	  << "runID: " << _run_id
	  << "spillID: " << _spill_count
	  <<endl;
	  cout <<endl;
	  return;
}
