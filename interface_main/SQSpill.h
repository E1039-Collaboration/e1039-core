/*
 * SQSpill.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw
 */

#ifndef _H_SQSpill_H_
#define _H_SQSpill_H_

#include <phool/PHObject.h>

#include <iostream>
#include <limits>
#include <string>
class SQStringMap;

/// An SQ interface class to hold the data of one spill.
class SQSpill : public PHObject {
public:
  virtual ~SQSpill() {}

  // PHObject virtual overloads

  virtual void         identify(std::ostream& os = std::cout) const {
    os << "---SQSpill base class------------" << std::endl;
  }
  virtual void         Reset() {};
  virtual int          isValid() const {return 0;}
  virtual SQSpill*        Clone() const {return NULL;}

  /// Return the run ID when this spill was taken.
  virtual int  get_run_id() const {return std::numeric_limits<int>::max();}
  virtual void set_run_id(const int a) {}

  /// Return the spill ID.
  virtual int  get_spill_id() const {return std::numeric_limits<int>::max();}
  virtual void set_spill_id(const int a) {}

  /// Return the target position in this spill.
  virtual short get_target_pos() const {return std::numeric_limits<short>::max();}
  virtual void  set_target_pos(const short a) {}

  /// [Obsolete] Use `SQHardSpill` instead.
  virtual int  get_bos_coda_id() const {return std::numeric_limits<int>::max();}
  virtual void set_bos_coda_id(const int a) {};

  /// [Obsolete] Use `SQHardSpill` instead.
  virtual int  get_bos_vme_time() const {return std::numeric_limits<int>::max();}
  virtual void set_bos_vme_time(const int a) {};

  /// [Obsolete] Use `SQHardSpill` instead.
  virtual int  get_eos_coda_id() const {return std::numeric_limits<int>::max();}
  virtual void set_eos_coda_id(const int a) {};

  /// [Obsolete] Use `SQHardSpill` instead.
  virtual int  get_eos_vme_time() const {return std::numeric_limits<int>::max();}
  virtual void set_eos_vme_time(const int a) {};

  /// Return the list of scaler variables read out at BOS.
  virtual SQStringMap* get_bos_scaler_list() { return 0; }

  /// Return the list of scaler variables read out at EOS.
  virtual SQStringMap* get_eos_scaler_list() { return 0; }

  /// Return the list of slow control variables.
  virtual SQStringMap* get_slow_cont_list() { return 0; }
  
protected:
  SQSpill() {}

private:

  ClassDef(SQSpill, 1);
};




#endif /* _H_SQSpill_H_ */
