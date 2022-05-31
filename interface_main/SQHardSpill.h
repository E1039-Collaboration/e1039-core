#ifndef _H_SQHardSpill_H_
#define _H_SQHardSpill_H_
#include <iostream>
#include <phool/PHObject.h>

/// An SQ interface class to hold the hardware-related data of one spill.
class SQHardSpill : public PHObject {
 protected:
  SQHardSpill() {;}

 public:
  virtual ~SQHardSpill() {;}

  virtual void Reset() = 0;
  virtual void identify(std::ostream& os=std::cout) const = 0;
  virtual int  isValid() const = 0;
  virtual SQHardSpill* clone() const = 0;
  virtual SQHardSpill* Clone() const = 0; ///< No use.  Only for backward compatibility.

  /// Return the Coda ID at BOS of this spill.
  virtual int  get_bos_coda_id() const = 0;
  virtual void set_bos_coda_id(const int a) = 0;

  /// Return the VME time at BOS of this spill.
  virtual int  get_bos_vme_time() const = 0;
  virtual void set_bos_vme_time(const int a) = 0;

  /// Return the Coda ID at EOS of this spill.
  virtual int  get_eos_coda_id() const = 0;
  virtual void set_eos_coda_id(const int a) = 0;

  /// Return the VME time at EOS of this spill.
  virtual int  get_eos_vme_time() const = 0;
  virtual void set_eos_vme_time(const int a) = 0;

  /// Return the time taken for the Coda input per spill.
  virtual double get_time_input() const = 0;
  virtual void   set_time_input(const double a) = 0;

  /// Return the time taken for the word decoding per spill.
  virtual double get_time_decode() const = 0;
  virtual void   set_time_decode(const double a) = 0;

  /// Return the time taken for the channel mapping per spill.
  virtual double get_time_map() const = 0;
  virtual void   set_time_map(const double a) = 0;

  /// Return the time taken for the SubsysReco event processing per spill.
  virtual double get_time_subsys() const = 0;
  virtual void   set_time_subsys(const double a) = 0;

  /// Return the time taken for the data output per spill.
  virtual double get_time_output() const = 0;
  virtual void   set_time_output(const double a) = 0;

  ClassDef(SQHardSpill, 1);
};

#endif /* _H_SQHardSpill_H_ */
