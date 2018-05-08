
#ifndef __PHFieldRegionalConst_H__
#define __PHFieldRegionalConst_H__

#include "PHField.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <map>
#include <string>
#include <vector>

class PHFieldRegionalConst : public PHField
{
  typedef boost::tuple<float, float> trio;

 public:
  PHFieldRegionalConst(const std::string &filename, const int verb = 0, const float magfield_rescale = 1.0);
  virtual ~PHFieldRegionalConst() {}
  //! access field value
  //! Follow the convention of G4ElectroMagneticField
  //! @param[in]  Point   space time coordinate. x, y, z, t in Geant4/CLHEP units
  //! @param[out] Bfield  field value. In the case of magnetic field, the order is Bx, By, Bz in in Geant4/CLHEP units
  void GetFieldValue(const double Point[4], double *Bfield) const;

 protected:

  double maxy_, miny_;
  double maxr_, minr_;
  double field_val_;
};

#endif  // __PHFieldRegionalConst_H
