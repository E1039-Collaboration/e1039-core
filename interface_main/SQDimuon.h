#ifndef _SQ_DIMUON__H_
#define _SQ_DIMUON__H_
#include <iostream>
#include <phool/PHObject.h>
#include <TLorentzVector.h>

/// An SQ interface class to hold one true or reconstructed dimuon.
class SQDimuon: public PHObject {
 public:
  virtual ~SQDimuon() {}

  virtual void identify(std::ostream& os = std::cout) const = 0;
  virtual void Reset() = 0;
  virtual int isValid() const = 0;
  virtual SQDimuon* Clone() const = 0;

  virtual int  get_dimuon_id() const = 0; ///< Return the dimuon ID, which is unique per event(?).
  virtual void set_dimuon_id(const int a) = 0;

  virtual int  get_rec_dimuon_id() const = 0; ///< Return the dimuon ID of associated reconstructed dimuon.  Valid only if this object holds truth dimuon info.
  virtual void set_rec_dimuon_id(const int a) = 0;

  virtual int  get_pdg_id() const = 0; ///< Return the GPD ID of parent particle.  It is valid only for true dimuon.
  virtual void set_pdg_id(const int a) = 0;

  virtual int  get_track_id_pos() const = 0; ///< Return the track ID of the positive track.
  virtual void set_track_id_pos(const int a) = 0;

  virtual int  get_track_id_neg() const = 0; ///< Return the track ID of the negative track.
  virtual void set_track_id_neg(const int a) = 0;

  virtual TVector3 get_pos() const = 0; ///< Return the dimuon position at vertex.
  virtual void     set_pos(const TVector3 a) = 0;

  virtual TLorentzVector get_mom() const = 0; ///< Return the dimuon momentum at vertex.
  virtual void           set_mom(const TLorentzVector a) = 0;

  virtual TLorentzVector get_mom_pos() const = 0; ///< Return the momentum of the positive track at vertex.
  virtual void           set_mom_pos(const TLorentzVector a) = 0;

  virtual TLorentzVector get_mom_neg() const = 0; ///< Return the momentum of the negative track at vertex.
  virtual void           set_mom_neg(const TLorentzVector a) = 0;

  virtual double get_mass() const = 0;
  virtual double get_x1()   const = 0;
  virtual double get_x2()   const = 0;
  virtual double get_xf()   const = 0;

  virtual double get_chisq() const = 0;

 protected:
  SQDimuon() {}

  ClassDef(SQDimuon, 1);
};

#endif // _SQ_DIMUON__H_
