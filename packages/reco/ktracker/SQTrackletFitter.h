#ifndef _SQTRACKLETFITTER_H
#define _SQTRACKLETFITTER_H

#include <Math/Factory.h>
#include <Math/Minimizer.h>
#include <Math/Functor.h>

#include "FastTracklet.h"

class SQTrackletFitter
{
public:
  SQTrackletFitter(bool KMAG_ON = true);
  ~SQTrackletFitter();

  /// core function - fit the tracklet and return the fit status
  int fit(Tracklet& tracklet);

  /// retrieve the fit parameter value and error
  double getParameter(int idx) { return minimizer->X()[idx]; }
  double getParaError(int idx) { return minimizer->Errors()[idx]; }

  /// retrieve the chi2
  double getChi2() { return minimizer->MinValue();}

private:
  /// The tracklet object
  Tracklet tracklet_fit;

  /// The fit parameter range
  double tx_max;
  double ty_max;
  double x0_max;
  double y0_max;
  double invP_min, invP_max;

  /// Least chi2 fitter and functor
  unsigned int nParameters;
  ROOT::Math::Minimizer* minimizer;
  ROOT::Math::Functor fcn;
};

#endif
