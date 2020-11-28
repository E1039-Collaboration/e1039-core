#ifndef __SQField3DCartesian_H__
#define __SQField3DCartesian_H__

#include "PHField.h"

#include <TVector3.h>

#include <vector>
#include <string>
#include <set>

class SQField3DCartesian: public PHField
{
public:
  SQField3DCartesian(const std::string& fname, const float magfield_rescale = 1.0);
  virtual ~SQField3DCartesian();

  //! access field value
  //! Follow the convention of G4ElectroMagneticField
  //! @param[in]  Point   space time coordinate. x, y, z, t in Geant4/CLHEP units
  //! @param[out] Bfield  field value. In the case of magnetic field, the order is Bx, By, Bz in in Geant4/CLHEP units
  void GetFieldValue(const double Point[4], double *Bfield) const;

  //! return the min and max in z
  double GetZMin() const { return zmin; }
  double GetZMax() const { return zmax; }

  //! return the index of the global index based on the local index
  int GetGlobalIndex(int xIdx, int yIdx, int zIdx) const;

protected:
  class FieldPoint
  {
  public:
    double x;
    double y;
    double z;
    TVector3 B;
  };

  std::string filename;
  std::set<double> xvals;
  std::set<double> yvals;
  std::set<double> zvals;
  std::vector<FieldPoint> fpoints;

  int xsteps;
  int ysteps;
  int zsteps;
  double xmin;
  double xmax;
  double ymin;
  double ymax;
  double zmin;
  double zmax;
  double xstepsize;
  double ystepsize;
  double zstepsize;

  double fieldstr;
};

#endif  // __SQField3D_H
