
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
  PHFieldRegionalConst(const double field = 5.0, const double magfield_rescale = 1.0, const int verb = 0);
  virtual ~PHFieldRegionalConst() {}
  //! access field value
  //! Follow the convention of G4ElectroMagneticField
  //! @param[in]  Point   space time coordinate. x, y, z, t in Geant4/CLHEP units
  //! @param[out] Bfield  field value. In the case of magnetic field, the order is Bx, By, Bz in in Geant4/CLHEP units
  void GetFieldValue(const double Point[4], double *Bfield) const;

  void identify(std::ostream& os = std::cout) const;

	double get_field_val() const {
		return _field_val;
	}

	void set_field_val(double fieldVal) {
		_field_val = fieldVal;
	}

	double get_maxr() const {
		return _maxr;
	}

	void set_maxr(double maxr) {
		_maxr = maxr;
	}

	double get_maxy() const {
		return _maxy;
	}

	void set_maxy(double maxy) {
		_maxy = maxy;
	}

	double get_mean_x() const {
		return _mean_x;
	}

	void set_mean_x(double meanX) {
		_mean_x = meanX;
	}

	double get_mean_y() const {
		return _mean_y;
	}

	void set_mean_y(double meanY) {
		_mean_y = meanY;
	}

	double get_mean_z() const {
		return _mean_z;
	}

	void set_mean_z(double meanZ) {
		_mean_z = meanZ;
	}

	double get_minr() const {
		return _minr;
	}

	void set_minr(double minr) {
		_minr = minr;
	}

	double get_miny() const {
		return _miny;
	}

	void set_miny(double miny) {
		_miny = miny;
	}

 protected:

  double _mean_x, _mean_y, _mean_z;
  double _maxy, _miny;
  double _maxr, _minr;
  double _field_val;
};

#endif  // __PHFieldRegionalConst_H
