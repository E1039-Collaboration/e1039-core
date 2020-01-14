#ifndef _SQ_MC_EVENT_V1__H_
#define _SQ_MC_EVENT_V1__H_
#include "SQMCEvent.h"

class SQMCEvent_v1 : public SQMCEvent {
 public:
  SQMCEvent_v1();
  virtual ~SQMCEvent_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int isValid() const { return 1; }
  SQMCEvent* Clone() const { return new SQMCEvent_v1(*this); }

  int  get_process_id() const { return _proc_id; }
  void set_process_id(const int a)   { _proc_id = a; }

  double get_cross_section() const  { return _xsec; }
  void   set_cross_section(const double a) { _xsec = a; }

  double get_weight() const  { return _weight; }
  void   set_weight(const double a) { _weight = a; }


  int  get_particle_id(const int i) const;
  void set_particle_id(const int i, const int a);

  TLorentzVector get_particle_momentum(const int i) const;
  void           set_particle_momentum(const int i, const TLorentzVector a);

 protected:
  static const int _N_PAR = 4; // 2 -> 2
  int _proc_id;
  double _xsec;
  double _weight;
  int _par_id[_N_PAR];
  TLorentzVector _par_mom[_N_PAR];

  ClassDef(SQMCEvent_v1, 1)
};

#endif // _SQ_MC_EVENT_V1__H_
