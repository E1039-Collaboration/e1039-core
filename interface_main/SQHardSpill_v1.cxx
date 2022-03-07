#include "SQHardSpill_v1.h"
using namespace std;

ClassImp(SQHardSpill_v1);

SQHardSpill_v1::SQHardSpill_v1()
  : _bos_coda_id  (0)
  , _bos_vme_time (0)
  , _eos_coda_id  (0)
  , _eos_vme_time (0)
  , _ts_deco_begin(TTimeStamp(0, 0))
  , _ts_deco_end  (TTimeStamp(0, 0))
  , _ts_proc_begin(TTimeStamp(0, 0))
  , _ts_proc_end  (TTimeStamp(0, 0))
{
  ;
}

void SQHardSpill_v1::Reset()
{
  _bos_coda_id      = 0;
  _bos_vme_time     = 0;
  _eos_coda_id      = 0;
  _eos_vme_time     = 0;
  _ts_deco_begin.SetSec    (0);
  _ts_deco_begin.SetNanoSec(0);
  _ts_deco_end  .SetSec    (0);
  _ts_deco_end  .SetNanoSec(0);
  _ts_proc_begin.SetSec    (0);
  _ts_proc_begin.SetNanoSec(0);
  _ts_proc_end  .SetSec    (0);
  _ts_proc_end  .SetNanoSec(0);
}

void SQHardSpill_v1::identify(ostream& os) const
{
  os << "---SQHardSpill_v1--------------------" << endl;
}
