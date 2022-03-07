#ifndef _H_SQHardEvent_H_
#define _H_SQHardEvent_H_
#include <iostream>
#include <phool/PHObject.h>

/// An SQ interface class to hold one hardware-related event info.
class SQHardEvent: public PHObject {
 protected: 
  SQHardEvent() {;}

 public:
  virtual ~SQHardEvent() {;}

  virtual void Reset() = 0;
  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual int  isValid() const = 0;
  virtual SQHardEvent* Clone() const = 0;
  
  /// Return the Coda-event ID, which is unique per run.
  virtual int  get_coda_event_id() const = 0;
  virtual void set_coda_event_id(const int a) = 0;
  
  /// Return the VME time.
  virtual int  get_vme_time() const = 0; 
  virtual void set_vme_time(const int a) = 0;
  
  /// Return the raw count of the selected trigger channel.
  virtual int  get_raw_matrix(const unsigned short i) const = 0;
  virtual void set_raw_matrix(const unsigned short i, const bool a) = 0;
  
  /// Return the after-inhibited count of the selected trigger channel.
  virtual int  get_after_inh_matrix(const unsigned short i) const = 0; 
  virtual void set_after_inh_matrix(const unsigned short i, const bool a) = 0;

  /// Return the quality flag of the V1495 readout.
  virtual short get_flag_v1495() const = 0;
  virtual void  set_flag_v1495(const short a) = 0;

  /// Return the number of QIE boards read out.
  virtual short get_n_board_qie() const = 0;
  virtual void  set_n_board_qie(const short a) = 0;
  
  /// Return the number of V1495 boards read out.
  virtual short get_n_board_v1495() const = 0;
  virtual void  set_n_board_v1495(const short a) = 0;
  
  /// Return the number of Taiwan-TDC boards read out.
  virtual short get_n_board_taiwan() const = 0;
  virtual void  set_n_board_taiwan(const short a) = 0;
  
  /// Return the number of trigger-bit boards read out.
  virtual short get_n_board_trig_bit() const = 0; 
  virtual void  set_n_board_trig_bit(const short a) = 0;
  
  /// Return the number of trigger-count boards read out.
  virtual short get_n_board_trig_count() const = 0;
  virtual void  set_n_board_trig_count(const short a) = 0;
  
  ClassDef(SQHardEvent, 1);
};

#endif /* _H_SQHardEvent_H_ */
