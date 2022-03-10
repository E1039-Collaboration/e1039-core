#ifndef _H_SQHardSpill_v1_H_
#define _H_SQHardSpill_v1_H_
#include <iostream>
#include "SQHardSpill.h"

class SQHardSpill_v1 : public SQHardSpill {
  int _bos_coda_id;
  int _bos_vme_time;
  int _eos_coda_id;
  int _eos_vme_time;
  TTimeStamp _ts_deco_begin;
  TTimeStamp _ts_deco_end  ;
  TTimeStamp _ts_proc_begin;
  TTimeStamp _ts_proc_end  ;

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
  
  virtual TTimeStamp get_timestamp_deco_begin() const      { return _ts_deco_begin; }
  virtual void       set_timestamp_deco_begin(const TTimeStamp a) { _ts_deco_begin = a; }

  virtual TTimeStamp get_timestamp_deco_end() const      { return _ts_deco_end; }
  virtual void       set_timestamp_deco_end(const TTimeStamp a) { _ts_deco_end = a; }

  virtual TTimeStamp get_timestamp_proc_begin() const      { return _ts_proc_begin; }
  virtual void       set_timestamp_proc_begin(const TTimeStamp a) { _ts_proc_begin = a; }

  virtual TTimeStamp get_timestamp_proc_end() const      { return _ts_proc_end; }
  virtual void       set_timestamp_proc_end(const TTimeStamp a) { _ts_proc_end = a; }

  ClassDef(SQHardSpill_v1, 1);
};


#endif /* _H_SQHardSpill_v1_H_ */
