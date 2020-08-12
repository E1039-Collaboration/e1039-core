#ifndef _SQ_DIMUON_V1__H_
#define _SQ_DIMUON_V1__H_
#include "SQDimuon.h"

class SQDimuon_v1 : public SQDimuon {
 public:
  SQDimuon_v1();
  virtual ~SQDimuon_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int isValid() const { return 1; }
  SQDimuon* Clone() const { return new SQDimuon_v1(*this); }

  int  get_dimuon_id() const { return _id; }
  void set_dimuon_id(const int a)   { _id = a; }

  virtual int  get_rec_dimuon_id() const { return _rec_id; }
  virtual void set_rec_dimuon_id(const int a) { _rec_id = a; }

  int  get_pdg_id() const { return _pdg_id; }
  void set_pdg_id(const int a)   { _pdg_id = a; }

  int  get_track_id_pos() const { return _track_id_pos; }
  void set_track_id_pos(const int a)   { _track_id_pos = a; }

  int  get_track_id_neg() const { return _track_id_neg; }
  void set_track_id_neg(const int a)   { _track_id_neg = a; }

  TVector3 get_pos() const    { return _pos; }
  void     set_pos(const TVector3 a) { _pos = a; }

  TLorentzVector get_mom() const          { return _mom; }
  void           set_mom(const TLorentzVector a) { _mom = a; }

  TLorentzVector get_mom_pos() const          { return _mom_pos; }
  void           set_mom_pos(const TLorentzVector a) { _mom_pos = a; }

  TLorentzVector get_mom_neg() const          { return _mom_neg; }
  void           set_mom_neg(const TLorentzVector a) { _mom_neg = a; }

  virtual double get_mass() const { return _mom.M(); }
  virtual double get_x1() const;
  virtual double get_x2() const;
  virtual double get_xf() const;

  virtual double get_chisq() const { return 0.; }

 protected:
  int _id;
  int _rec_id;
  int _pdg_id;
  int _track_id_pos;
  int _track_id_neg;
  TVector3 _pos;
  TLorentzVector _mom;
  TLorentzVector _mom_pos;
  TLorentzVector _mom_neg;

  ClassDef(SQDimuon_v1, 1);
};

//struct SQDimuonVector : public PHObject, public std::vector<SQDimuon> {
//  SQDimuonVector() {;}
//  virtual ~SQDimuonVector() {;}
//
//  void identify(std::ostream& os = std::cout) const {;}
//  void Reset() { *this = SQDimuonVector(); }
//  int isValid() const { return 1; }
//  SQDimuonVector* Clone() const { return new SQDimuonVector(*this); }
//
//  ClassDef(SQDimuonVector, 1);
//};

#endif // _SQ_DIMUON_V1__H_
