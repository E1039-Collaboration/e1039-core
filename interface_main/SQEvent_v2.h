#ifndef _H_SQEvent_v2_H_
#define _H_SQEvent_v2_H_
#include <map>
#include <iostream>
#include <limits>
#include <climits>
#include "SQEvent.h"

class SQEvent_v2: public SQEvent {
  int _run_id;
  int _spill_id;
  int _event_id;
  unsigned short _trigger; //< NIM[1-5], MATRIX[1-5]
  int _data_quality;
  
  int _qie_presums[4];
  int _qie_trig_cnt;
  int _qie_turn_id;
  int _qie_rf_id;
  int _qie_rf_inte[33];

 public:
  SQEvent_v2();
  virtual ~SQEvent_v2();
  
  virtual void Reset();
  virtual void identify(std::ostream& os = std::cout) const;
  virtual int  isValid() const {return 1;}
  virtual SQEvent* Clone() const {return new SQEvent_v2(*this);}
  
  virtual int  get_run_id() const {return _run_id;}
  virtual void set_run_id(const int a) {_run_id = a;}
  
  virtual int  get_spill_id() const {return _spill_id;}
  virtual void set_spill_id(const int a) {_spill_id = a;}
  
  virtual int  get_event_id() const {return _event_id;}
  virtual void set_event_id(const int a) {_event_id = a;}

  virtual int  get_data_quality() const {return _data_quality;}
  virtual void set_data_quality(const int a) {_data_quality = a;}

  virtual bool get_trigger(const SQEvent::TriggerMask i) const {return (_trigger&(1<<i)) > 0 ;}
  virtual void set_trigger(const SQEvent::TriggerMask i, const bool a) {a ? (_trigger |= (1<<i)) : (_trigger &= ~(1<<i)) ;}
  
  virtual unsigned short get_trigger() const {return _trigger;}
  virtual void           set_trigger(const unsigned short a) {_trigger = a;}
  
  virtual int  get_qie_presum(const unsigned short i) const {
    if (i<4) return _qie_presums[i]; 
    return INT_MAX;
  }
  virtual void set_qie_presum(const unsigned short i, const int a) {
    if(i<4) _qie_presums[i] = a;
    else std::cout<<"SQEvent_v2::set_qie_presum: i>=4";
  }
  
  virtual int  get_qie_trigger_count() const { return _qie_trig_cnt; }
  virtual void set_qie_trigger_count(const int a)   { _qie_trig_cnt = a; }
  
  virtual int  get_qie_turn_id() const { return _qie_turn_id; }
  virtual void set_qie_turn_id(const int a)   { _qie_turn_id = a; }
  
  virtual int  get_qie_rf_id() const { return _qie_rf_id; }
  virtual void set_qie_rf_id(const int a)   { _qie_rf_id = a; }
  
  virtual int  get_qie_rf_intensity(const short i) const {
    if (abs(i)<=16) return _qie_rf_inte[i+16]; 
    return INT_MAX;
  }
  virtual void set_qie_rf_intensity(const short i, const int a) {
    if(abs(i)<=16) _qie_rf_inte[i+16] = a;
    else std::cout<<"SQEvent_v2::set_qie_rf_intensity: abs(i)>16";
  }

  ClassDef(SQEvent_v2, 1);
};

#endif /* _H_SQEvent_v2_H_ */
