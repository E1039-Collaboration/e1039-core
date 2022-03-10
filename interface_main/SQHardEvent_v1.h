#ifndef _H_SQHardEvent_v1_H_
#define _H_SQHardEvent_v1_H_
#include <map>
#include <iostream>
//#include <limits>
//#include <climits>
#include "SQHardEvent.h"

class SQHardEvent_v1: public SQHardEvent {
  int _coda_event_id;
  int _vme_time;
  int _raw_matrix[5];
  int _after_inh_matrix[5];

  short _flag_v1495;
  short _n_board_qie;
  short _n_board_v1495;
  short _n_board_taiwan;
  short _n_board_trig_b;
  short _n_board_trig_c;
  
 public:
  SQHardEvent_v1();
  virtual ~SQHardEvent_v1();

  virtual void Reset();
  virtual void identify(std::ostream& os = std::cout) const;
  virtual int  isValid() const { return 1; }
  virtual SQHardEvent* Clone() const { return new SQHardEvent_v1(*this); }

  virtual int  get_coda_event_id() const { return _coda_event_id; }
  virtual void set_coda_event_id(const int a)   { _coda_event_id = a; }

  virtual int  get_vme_time() const { return _vme_time; }
  virtual void set_vme_time(const int a)   { _vme_time = a; }

  virtual int  get_raw_matrix(const unsigned short i) const;
  virtual void set_raw_matrix(const unsigned short i, const bool a);

  virtual int  get_after_inh_matrix(const unsigned short i) const;
  virtual void set_after_inh_matrix(const unsigned short i, const bool a);

  virtual short get_flag_v1495() const { return _flag_v1495; }
  virtual void  set_flag_v1495(const short a) { _flag_v1495 = a; }

  virtual short get_n_board_qie() const { return _n_board_qie; }
  virtual void  set_n_board_qie(const short a) { _n_board_qie = a; }
  
  virtual short get_n_board_v1495() const { return _n_board_v1495; }
  virtual void  set_n_board_v1495(const short a) { _n_board_v1495 = a; }
  
  virtual short get_n_board_taiwan() const { return _n_board_taiwan; }
  virtual void  set_n_board_taiwan(const short a) { _n_board_taiwan = a; }
  
  virtual short get_n_board_trig_bit() const { return _n_board_trig_b; }
  virtual void  set_n_board_trig_bit(const short a) { _n_board_trig_b = a; }
  
  virtual short get_n_board_trig_count() const { return _n_board_trig_c; }
  virtual void  set_n_board_trig_count(const short a) { _n_board_trig_c = a; }

  ClassDef(SQHardEvent_v1, 1);
};

#endif /* _H_SQHardEvent_v1_H_ */
