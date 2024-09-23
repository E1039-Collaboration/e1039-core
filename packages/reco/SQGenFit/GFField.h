#ifndef _GFFIELD_H
#define _GFFIELD_H

#include <GenFit/AbsBField.h>
#include <TVector3.h>

#include <phfield/PHField.h>

namespace SQGenFit
{
class GFField: public genfit::AbsBField
{
public:
  GFField(const PHField* field);
  virtual ~GFField() {}

  TVector3 get(const TVector3& pos) const;
  void get(const double& posX, const double& posY, const double& posZ, double& Bx, double& By, double& Bz) const;

  void setScale(double scale   = 1.) { _scale = scale; }
  void setOffset(double offset = 0.) { _offset = offset; }
  void disable() { _disable = true; }

private:
  const PHField* _field;
  double _scale;
  double _offset;
  bool   _disable;

};
}

#endif