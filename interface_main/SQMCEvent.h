#ifndef _SQ_MC_EVENT__H_
#define _SQ_MC_EVENT__H_
#include <iostream>
#include <phool/PHObject.h>
#include <TLorentzVector.h>

/// An SQ interface class to hold one simulated-event header.
/**
 * It holds a set of variables that are peculiar to _simulated_ event.
 * All other variables that are common to _real_ event are held by SQEvent.
 */
class SQMCEvent: public PHObject {
 public:
  virtual ~SQMCEvent() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int isValid() const = 0;
  virtual SQMCEvent* Clone() const = 0;

  virtual int  get_process_id() const = 0; ///< Return the primary process ID.
  virtual void set_process_id(const int a) = 0;

  virtual double get_cross_section() const = 0; ///< Return the cross section.
  virtual void   set_cross_section(const double a) = 0;

  virtual double get_weight() const = 0; ///< Return the event weight.
  virtual void   set_weight(const double a) = 0;

  virtual int  get_particle_id(const int i) const = 0; ///< Return the particle ID of the primary process, where i=0...3 for "0 + 1 -> 2 + 3".
  virtual void set_particle_id(const int i, const int a) = 0;

  virtual TLorentzVector get_particle_momentum(const int i) const = 0; ///< Return the particle momentum of the primary process, where i=0...3 for "0 + 1 -> 2 + 3".
  virtual void           set_particle_momentum(const int i, const TLorentzVector a) = 0;

 protected:
  SQMCEvent() {}

  ClassDef(SQMCEvent, 1);
};

#endif // _SQ_MC_EVENT__H_
