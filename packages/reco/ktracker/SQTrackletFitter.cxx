#include "SQTrackletFitter.h"

#include <phool/recoConsts.h>

SQTrackletFitter::SQTrackletFitter(bool KMAG_ON)
{
  nParameters = KMAG_ON ? 5 : 4;

  recoConsts* rc = recoConsts::instance();
  tx_max = rc->get_DoubleFlag("TX_MAX");
  ty_max = rc->get_DoubleFlag("TY_MAX");
  x0_max = rc->get_DoubleFlag("X0_MAX");
  y0_max = rc->get_DoubleFlag("Y0_MAX");
  invP_min = rc->get_DoubleFlag("INVP_MIN");
  invP_max = rc->get_DoubleFlag("INVP_MAX");

  minimizer = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Combined");
  fcn = ROOT::Math::Functor(&tracklet_fit, &Tracklet::Eval, nParameters);

  minimizer->SetMaxFunctionCalls(1000000);
  minimizer->SetMaxIterations(100);
  minimizer->SetTolerance(1E-2);
  minimizer->SetFunction(fcn);
  minimizer->SetPrintLevel(0);
}

SQTrackletFitter::~SQTrackletFitter()
{
  delete minimizer;
}

int SQTrackletFitter::fit(Tracklet& tracklet)
{
  tracklet_fit = tracklet;

  minimizer->SetLimitedVariable(0, "tx", tracklet.tx, 0.001, -tx_max, tx_max);
  minimizer->SetLimitedVariable(1, "ty", tracklet.ty, 0.001, -ty_max, ty_max);
  minimizer->SetLimitedVariable(2, "x0", tracklet.x0, 0.1, -x0_max, x0_max);
  minimizer->SetLimitedVariable(3, "y0", tracklet.y0, 0.1, -y0_max, y0_max);
  if(nParameters == 5)
  {
    minimizer->SetLimitedVariable(4, "invP", tracklet.invP, 0.001*tracklet.invP, invP_min, invP_max);
  }

  minimizer->Minimize();

  //update the track parameters
  tracklet.tx = minimizer->X()[0];
  tracklet.ty = minimizer->X()[1];
  tracklet.x0 = minimizer->X()[2];
  tracklet.y0 = minimizer->X()[3];

  tracklet.err_tx = minimizer->Errors()[0];
  tracklet.err_ty = minimizer->Errors()[1];
  tracklet.err_x0 = minimizer->Errors()[2];
  tracklet.err_y0 = minimizer->Errors()[3];

  /// Avoid too-small error, which causes NaN in Tracklet::operator+().
  if (tracklet.err_tx < 1e-6) tracklet.err_tx = 1e-6;
  if (tracklet.err_ty < 1e-6) tracklet.err_ty = 1e-6;
  if (tracklet.err_x0 < 1e-4) tracklet.err_x0 = 1e-4;
  if (tracklet.err_y0 < 1e-4) tracklet.err_y0 = 1e-4;

  if(nParameters == 5)
  {
    tracklet.invP = minimizer->X()[4];
    tracklet.err_invP = minimizer->Errors()[4];
  }

  tracklet.chisq = minimizer->MinValue();
  return minimizer->Status();
}