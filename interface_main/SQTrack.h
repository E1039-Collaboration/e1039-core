#ifndef _SQ_TRACK__H_
#define _SQ_TRACK__H_
#include <iostream>
#include <phool/PHObject.h>
#include <TLorentzVector.h>

/// An SQ interface class to hold one true or reconstructed track.
class SQTrack: public PHObject {
 public:
  virtual ~SQTrack() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int isValid() const = 0;
  virtual SQTrack* Clone() const = 0;

  virtual int  get_track_id() const = 0; ///< Return the track ID, which is unique per event(?).
  virtual void set_track_id(const int a) = 0;

  virtual int  get_charge() const = 0; ///< Return the charge, i.e. +1 or -1.
  virtual void set_charge(const int a) = 0;

  virtual int  get_num_hits() const = 0; ///< Return the number of hits associated to this track.
  virtual void set_num_hits(const int a) = 0;

  virtual TVector3 get_pos_vtx() const = 0; ///< Return the track position at vertex.
  virtual void     set_pos_vtx(const TVector3 a) = 0;

  virtual TVector3 get_pos_st1() const = 0; ///< Return the track position at Station 1.
  virtual void     set_pos_st1(const TVector3 a) = 0;

  virtual TVector3 get_pos_st3() const = 0; ///< Return the track position at Station 3.
  virtual void     set_pos_st3(const TVector3 a) = 0;

  virtual TLorentzVector get_mom_vtx() const = 0; ///< Return the track momentum at vertex.
  virtual void           set_mom_vtx(const TLorentzVector a) = 0;

  virtual TLorentzVector get_mom_st1() const = 0; ///< Return the track momentum at Station 1.
  virtual void           set_mom_st1(const TLorentzVector a) = 0;

  virtual TLorentzVector get_mom_st3() const = 0; ///< Return the track momentum at Station 3.
  virtual void           set_mom_st3(const TLorentzVector a) = 0;

 protected:
  SQTrack() {}

  ClassDef(SQTrack, 1);
};

#endif // _SQ_TRACK__H_
