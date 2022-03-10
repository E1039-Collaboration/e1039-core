#include "SQEvent_v2.h"
using namespace std;

ClassImp(SQEvent_v2)

SQEvent_v2::SQEvent_v2()
  : _run_id      (INT_MAX)
  , _spill_id    (INT_MAX)
  , _event_id    (INT_MAX)
  , _trigger     (0)
  , _data_quality(INT_MAX)
  , _qie_trig_cnt(INT_MAX) 
  , _qie_turn_id (INT_MAX) 
  , _qie_rf_id   (INT_MAX) 
{
  memset(_qie_presums, 0, sizeof(_qie_presums));
  memset(_qie_rf_inte, 0, sizeof(_qie_rf_inte));
}

SQEvent_v2::~SQEvent_v2()
{
  Reset();
}

void SQEvent_v2::Reset()
{
  _run_id       = INT_MAX;
  _spill_id     = INT_MAX;
  _event_id     = INT_MAX;
  _trigger      = 0;
  _data_quality = INT_MAX;
  _qie_trig_cnt = INT_MAX;
  _qie_turn_id  = INT_MAX;
  _qie_rf_id    = INT_MAX;
  memset(_qie_presums, 0, sizeof(_qie_presums));
  memset(_qie_rf_inte, 0, sizeof(_qie_rf_inte));
}

void SQEvent_v2::identify(std::ostream& os) const
{
  os << "---SQEvent_v2::identify:--------------------------\n"
     << "runID: " << _run_id << " spillID: " << _spill_id << " eventID: " << _event_id << "\n";
  for(int i = SQEvent::NIM1; i<=SQEvent::NIM5; ++i) {
    int name = i - SQEvent::NIM1 + 1;
    os <<"NIM"<<name<<": " << get_trigger(static_cast<SQEvent::TriggerMask>(i)) << "; ";
  }
  os <<endl;
  for(int i = SQEvent::MATRIX1; i<=SQEvent::MATRIX5; ++i){
    int name = i - SQEvent::MATRIX1 + 1;
    os <<"MATRIX"<<name<<": " << get_trigger(static_cast<SQEvent::TriggerMask>(i)) << "; ";
  }
  os <<endl;
}
