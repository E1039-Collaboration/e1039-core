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

  void setScale(double scale) { _scale = scale; }
  void disable() { _disable = true; }

private:
  const PHField* _field;
  double _scale;
  bool   _disable;

};
}

#endif