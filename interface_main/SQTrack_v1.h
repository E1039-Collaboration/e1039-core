#ifndef _SQ_TRACK_V1__H_
#define _SQ_TRACK_V1__H_
#include "SQTrack.h"

class SQTrack_v1 : public SQTrack {
 public:
  SQTrack_v1();
  virtual ~SQTrack_v1();

  void identify(std::ostream& os = std::cout) const;
  void Reset();
  int isValid() const { return 1; }
  SQTrack* Clone() const { return new SQTrack_v1(*this); }

  virtual int  get_track_id() const { return _id; }
  virtual void set_track_id(const int a)   { _id = a; }

  virtual int  get_rec_track_id() const { return _rec_id; }
  virtual void set_rec_track_id(const int a)   { _rec_id = a; }

  virtual int  get_charge() const { return _charge; }
  virtual void set_charge(const int a)   { _charge = a; }

  virtual int  get_num_hits() const { return _n_hits; }
  virtual void set_num_hits(const int a)   { _n_hits = a; }

  virtual TVector3 get_pos_vtx() const    { return _pos_vtx; }
  virtual void     set_pos_vtx(const TVector3 a) { _pos_vtx = a; }

  virtual TVector3 get_pos_st1() const    { return _pos_st1; }
  virtual void     set_pos_st1(const TVector3 a) { _pos_st1 = a; }

  virtual TVector3 get_pos_st3() const    { return _pos_st3; }
  virtual void     set_pos_st3(const TVector3 a) { _pos_st3 = a; }

  virtual TLorentzVector get_mom_vtx() const          { return _mom_vtx; }
  virtual void           set_mom_vtx(const TLorentzVector a) { _mom_vtx = a; }

  virtual TLorentzVector get_mom_st1() const          { return _mom_st1; }
  virtual void           set_mom_st1(const TLorentzVector a) { _mom_st1 = a; }

  virtual TLorentzVector get_mom_st3() const          { return _mom_st3; }
  virtual void           set_mom_st3(const TLorentzVector a) { _mom_st3 = a; }

  //reconstruction-related functions - all returns 0
  virtual double get_chisq() const          { return 0.; }
  virtual double get_chisq_target() const   { return 0.; }
  virtual double get_chisq_dump() const     { return 0.; }
  virtual double get_chsiq_upstream() const { return 0.; }
  
  virtual TVector3 get_pos_target() const   { return TVector3(0., 0., 0.); }
  virtual TVector3 get_pos_dump() const     { return TVector3(0., 0., 0.); }

  virtual TLorentzVector get_mom_target() const   { return TLorentzVector(0., 0., 0., 0.); }
  virtual TLorentzVector get_mom_dump() const     { return TLorentzVector(0., 0., 0., 0.); }

  virtual int get_hit_id(const int i) const { return 0; } 

 protected:
  int _id;
  int _rec_id;
  int _charge;
  int _n_hits;
  TVector3 _pos_vtx;
  TVector3 _pos_st1;
  TVector3 _pos_st3;
  TLorentzVector _mom_vtx;
  TLorentzVector _mom_st1;
  TLorentzVector _mom_st3;

  ClassDef(SQTrack_v1, 1);
};

//struct SQTrackVector : public PHObject, public std::vector<SQTrack> {
//  SQTrackVector() {;}
//  virtual ~SQTrackVector() {;}
//
//  void identify(std::ostream& os = std::cout) const {;}
//  void Reset() { *this = SQTrackVector(); }
//  int isValid() const { return 1; }
//  SQTrackVector* Clone() const { return new SQTrackVector(*this); }
//
//  ClassDef(SQTrackVector, 1);
//};

#endif // _SQ_TRACK_V1__H_
