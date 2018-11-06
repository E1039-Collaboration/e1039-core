/*
 * SQSpill_v2.h
 */
#ifndef _H_SQSpill_v2_H_
#define _H_SQSpill_v2_H_
#include <phool/PHObject.h>
#include <iostream>
#include "SQSpill.h"
class SQStringMap;

class SQSpill_v2 : public SQSpill {

public:

  SQSpill_v2();
  virtual ~SQSpill_v2();

  // PHObject virtual overloads

  void         identify(std::ostream& os = std::cout) const;
  void         Reset() {*this = SQSpill_v2();}
  int          isValid() const;
  SQSpill*        Clone() const {return (new SQSpill_v2(*this));}

  virtual int          get_run_id() const                               {return _run_id;}
  virtual void         set_run_id(const int a)                          {_run_id = a;}

  virtual int          get_spill_id() const                               {return _spill_id;}
  virtual void         set_spill_id(const int a)                          {_spill_id = a;}

  virtual short        get_target_pos() const                               {return _target_pos;}
  virtual void         set_target_pos(const short a)                          {_target_pos = a;}

  virtual int  get_bos_coda_id() const { return _bos_coda_id; }
  virtual void set_bos_coda_id(const int a) { _bos_coda_id = a; }

  virtual int  get_bos_vme_time() const {return _bos_vme_time; }
  virtual void set_bos_vme_time(const int a) { _bos_vme_time = a; }

  virtual int  get_eos_coda_id() const {return _eos_coda_id; }
  virtual void set_eos_coda_id(const int a) { _eos_coda_id = a; }

  virtual int  get_eos_vme_time() const {return _eos_vme_time; }
  virtual void set_eos_vme_time(const int a) { _eos_vme_time = a; }
  
  virtual SQStringMap* get_bos_scaler_list() { return _bos_scaler_list; }
  virtual SQStringMap* get_eos_scaler_list() { return _eos_scaler_list; }

  virtual SQStringMap* get_slow_cont_list() { return _slow_cont_list; }

private:

  int _run_id;
  int _spill_id;
  short _target_pos;
  int _bos_coda_id;
  int _bos_vme_time;
  int _eos_coda_id;
  int _eos_vme_time;
  SQStringMap* _bos_scaler_list;
  SQStringMap* _eos_scaler_list;
  SQStringMap* _slow_cont_list;

  ClassDef(SQSpill_v2, 1);
};


#endif /* _H_SQSpill_v2_H_ */
