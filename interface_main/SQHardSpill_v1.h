#ifndef _H_SQHardSpill_v1_H_
#define _H_SQHardSpill_v1_H_
#include <iostream>
#include "SQHardSpill.h"

class SQHardSpill_v1 : public SQHardSpill {
  int _bos_coda_id;
  int _bos_vme_time;
  int _eos_coda_id;
  int _eos_vme_time;
  double _time_input;
  double _time_decode;
  double _time_map;
  double _time_subsys;
  double _time_output;

 public:
  SQHardSpill_v1();
  virtual ~SQHardSpill_v1() {;}

  SQHardSpill* clone() const { return new SQHardSpill_v1(*this); }
  SQHardSpill* Clone() const { return new SQHardSpill_v1(*this); }
  void Reset();
  int isValid() const { return 1; }
  void identify(std::ostream& os = std::cout) const;

  virtual int  get_bos_coda_id() const { return _bos_coda_id; }
  virtual void set_bos_coda_id(const int a)   { _bos_coda_id = a; }

  virtual int  get_bos_vme_time() const { return _bos_vme_time; }
  virtual void set_bos_vme_time(const int a)   { _bos_vme_time = a; }

  virtual int  get_eos_coda_id() const { return _eos_coda_id; }
  virtual void set_eos_coda_id(const int a)   { _eos_coda_id = a; }

  virtual int  get_eos_vme_time() const { return _eos_vme_time; }
  virtual void set_eos_vme_time(const int a)   { _eos_vme_time = a; }
  
  virtual double get_time_input() const  { return _time_input; }
  virtual void   set_time_input(const double a) { _time_input = a; }

  virtual double get_time_decode() const  { return _time_decode; }
  virtual void   set_time_decode(const double a) { _time_decode = a; }

  virtual double get_time_map() const  { return _time_map; }
  virtual void   set_time_map(const double a) { _time_map = a; }

  virtual double get_time_subsys() const  { return _time_subsys; }
  virtual void   set_time_subsys(const double a) { _time_subsys = a; }

  virtual double get_time_output() const  { return _time_output; }
  virtual void   set_time_output(const double a) { _time_output = a; }

  ClassDef(SQHardSpill_v1, 1);
};


#endif /* _H_SQHardSpill_v1_H_ */
