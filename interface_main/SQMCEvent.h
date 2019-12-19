#ifndef _SQ_MC_EVENT__H_
#define _SQ_MC_EVENT__H_
#include <iostream>
#include <phool/PHObject.h>
#include <TLorentzVector.h>

class SQMCEvent: public PHObject {
 public:
  virtual ~SQMCEvent() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int isValid() const = 0;
  virtual SQMCEvent* Clone() const = 0;

  virtual int  get_process_id() const = 0;
  virtual void set_process_id(const int a) = 0;

  virtual double get_cross_section() const = 0;
  virtual void   set_cross_section(const double a) = 0;

  virtual int  get_particle_id(const int i) const = 0;
  virtual void set_particle_id(const int i, const int a) = 0;

  virtual TLorentzVector get_particle_momentum(const int i) const = 0;
  virtual void           set_particle_momentum(const int i, const TLorentzVector a) = 0;

 protected:
  SQMCEvent() {}

  ClassDef(SQMCEvent, 1);
};

#endif // _SQ_MC_EVENT__H_
