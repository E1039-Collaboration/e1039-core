#ifndef _H_SQHardSpill_H_
#define _H_SQHardSpill_H_
#include <iostream>
#include <TTimeStamp.h>
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

  /// Return the times when the decoding of spill begins.
  virtual TTimeStamp get_timestamp_deco_begin() const = 0;
  virtual void       set_timestamp_deco_begin(const TTimeStamp a) = 0;

  /// Return the time when the decoding of spill ends.
  virtual TTimeStamp get_timestamp_deco_end() const = 0;
  virtual void       set_timestamp_deco_end(const TTimeStamp a) = 0;

  /// Return the UNIX time when the event process of spill begins.
  virtual TTimeStamp get_timestamp_proc_begin() const = 0;
  virtual void       set_timestamp_proc_begin(const TTimeStamp a) = 0;

  /// Return the UNIX time when the event process of spill ends.
  virtual TTimeStamp get_timestamp_proc_end() const = 0;
  virtual void       set_timestamp_proc_end(const TTimeStamp a) = 0;

  ClassDef(SQHardSpill, 1);
};

#endif /* _H_SQHardSpill_H_ */
