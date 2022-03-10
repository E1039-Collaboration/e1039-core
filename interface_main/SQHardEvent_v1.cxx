#include <climits>
#include "SQHardEvent_v1.h"
using namespace std;

ClassImp(SQHardEvent_v1)

SQHardEvent_v1::SQHardEvent_v1()
  : _coda_event_id (INT_MAX)
  , _vme_time      (INT_MAX)
  , _flag_v1495    (SHRT_MAX) 
  , _n_board_qie   (SHRT_MAX)
  , _n_board_v1495 (SHRT_MAX)
  , _n_board_taiwan(SHRT_MAX)
  , _n_board_trig_b(SHRT_MAX)
  , _n_board_trig_c(SHRT_MAX)
{
  memset(_raw_matrix      , 0, sizeof(_raw_matrix));
  memset(_after_inh_matrix, 0, sizeof(_after_inh_matrix));
}

SQHardEvent_v1::~SQHardEvent_v1()
{
  Reset();
}

void SQHardEvent_v1::Reset()
{
  _coda_event_id  = INT_MAX;
  _vme_time       = INT_MAX;
  _flag_v1495     = SHRT_MAX;
  _n_board_qie    = SHRT_MAX;
  _n_board_v1495  = SHRT_MAX;
  _n_board_taiwan = SHRT_MAX;
  _n_board_trig_b = SHRT_MAX;
  _n_board_trig_c = SHRT_MAX;
  memset(_raw_matrix      , 0, sizeof(_raw_matrix));
  memset(_after_inh_matrix, 0, sizeof(_after_inh_matrix));
}

int SQHardEvent_v1::get_raw_matrix(const unsigned short i) const
{
  if (i < 5) return _raw_matrix[i];
  return INT_MAX;
}

void SQHardEvent_v1::set_raw_matrix(const unsigned short i, const bool a)
{
  if (i < 5) _raw_matrix[i] = a;
  else cout << "SQHardEvent_v1::set_raw_matrix: i>=5" << endl;
}

int SQHardEvent_v1::get_after_inh_matrix(const unsigned short i) const
{
  if (i < 5) return _after_inh_matrix[i];
  return INT_MAX;
}

void SQHardEvent_v1::set_after_inh_matrix(const unsigned short i, const bool a)
{
  if (i < 5) _after_inh_matrix[i] = a;
  else cout << "SQHardEvent_v1::set_after_inh_matrix: i>=5" << endl;
}

void SQHardEvent_v1::identify(std::ostream& os) const
{
  os << "---SQHardEvent_v1::identify:--------------------------\n"
     << "codaEventID: " << _coda_event_id << "\n"
     << "raw_matrix: ";
  for (int i = 0; i < 5; ++i) os << " " << _raw_matrix[i];
  os << "\n"
     << "after_inh_matrix: ";
  for (int i = 0; i < 5; ++i) os << " " << _after_inh_matrix[i];
  os << endl;
}
