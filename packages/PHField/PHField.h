#ifndef __PHFIELD_H__
#define __PHFIELD_H__

#include <phool/PHObject.h>

// units of this class. To convert internal value to Geant4/CLHEP units for fast access
//#include <CLHEP/Units/SystemOfUnits.h>
#include <Geant4/G4SystemOfUnits.hh>

#include <vector>

//! \brief transient DST object for field storage and access
class PHField : public PHObject
{
 public:
  //! constructor
  explicit PHField(const int verb = 0)
    : verb_(verb)
  {
  }
  virtual ~PHField() {}
  //! access field value
  //! Follow the convention of G4ElectroMagneticField
  //! @param[in]  Point   space time coordinate. x, y, z, t in Geant4/CLHEP units
  //! @param[out] Bfield  field value. In the case of magnetic field, the order is Bx, By, Bz in in Geant4/CLHEP units
  virtual void GetFieldValue(
      const double Point[4],
      double *Bfield) const = 0;

  virtual void identify(std::ostream& os = std::cout) const {std::cout << "I am a PHField object!" << std::endl;}

  void Verbosity(const int i) { verb_ = i; }
 protected:
  unsigned verb_;

};

#endif
