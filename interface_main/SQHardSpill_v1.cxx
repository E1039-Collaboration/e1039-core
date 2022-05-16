#include "SQHardSpill_v1.h"
using namespace std;

ClassImp(SQHardSpill_v1);

SQHardSpill_v1::SQHardSpill_v1()
  : _bos_coda_id  (0)
  , _bos_vme_time (0)
  , _eos_coda_id  (0)
  , _eos_vme_time (0)
  , _time_input   (0)
  , _time_decode  (0)
  , _time_map     (0)
  , _time_subsys  (0)
  , _time_output  (0)
{
  ;
}

void SQHardSpill_v1::Reset()
{
  _bos_coda_id  = 0;
  _bos_vme_time = 0;
  _eos_coda_id  = 0;
  _eos_vme_time = 0;
  _time_input   = 0;
  _time_decode  = 0;
  _time_map     = 0;
  _time_subsys  = 0;
  _time_output  = 0;
}

void SQHardSpill_v1::identify(ostream& os) const
{
  os << "---SQHardSpill_v1--------------------" << endl;
}
