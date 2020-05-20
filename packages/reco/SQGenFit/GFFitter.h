#ifndef _GFFITTER_H
#define _GFFITTER_H

#include <GenFit/AbsBField.h>
#include <GenFit/AbsKalmanFitter.h>
#include <GenFit/EventDisplay.h>
#include <TString.h>
#include <TGeoManager.h>

#include "GFTrack.h"
#include "GFField.h"

namespace SQGenFit
{
class GFFitter
{
public:
  GFFitter();
  ~GFFitter();

  void setVerbosity(unsigned int v);

  void init(GFField* field, const TString& fitter_choice = "KalmanFitterRefTrack");
  int processTrack(GFTrack& track, bool display = false);

  const TString& getFitterType() const { return _fitterTy; }
  genfit::AbsKalmanFitter* getGenFitFitter() { return _kmfitter; }

  void displayEvent();

private:
  TString _fitterTy;
  genfit::AbsKalmanFitter* _kmfitter;
  unsigned int _verbosity;

  genfit::EventDisplay* _display;
};
}

#endif