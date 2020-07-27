/*
kalmanFilter.cxx

Implimentation of the class KalmanFilter, provides all core
functionalities of the track fitting procedure

Author: Kun Liu, liuk@fnal.gov
Created: 11-21-2011
*/

#include <iostream>
#include <algorithm>
#include <cmath>

#include <phool/recoConsts.h>

#include "KalmanFilter.h"

KalmanFilter* KalmanFilter::p_kmfit = nullptr;

KalmanFilter* KalmanFilter::instance()
{
    if(p_kmfit == nullptr)
    {
        p_kmfit = new KalmanFilter();
    }

    return p_kmfit;
}

void KalmanFilter::close()
{
    if(p_kmfit != nullptr)
    {
        delete p_kmfit;
    }
}

KalmanFilter::KalmanFilter(bool limitedStep)
{
//    JobOptsSvc* p_jobOptsSvc = JobOptsSvc::instance();
//    _extrapolator.init(p_jobOptsSvc);
}

bool KalmanFilter::initExtrapolator(const PHField *field,  const TGeoManager *geom)
{
	_extrapolator.init(field, geom);
}

bool KalmanFilter::fit_node(Node& _node)
{
    if(predict(_node) && filter(_node))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool KalmanFilter::predict(Node& _node)
{
    if(_node.isPredictionDone())
    {
        LogInfo("In prediction: Prediction already done for this node!");
        return false;
    }

    if(fabs(_trkpar_curr._state_kf[0][0]) < 1E-5 || fabs(_trkpar_curr._state_kf[0][0]) > 1. || _trkpar_curr._state_kf[0][0] != _trkpar_curr._state_kf[0][0])
    {
        LogInfo("Current track parameter is undefined! ");
        //_node.getHit().print();
        //_node.print(true);
        return false;
    }

    double z_pred = _node.getZ();
    _extrapolator.setInitialStateWithCov(_trkpar_curr._z, _trkpar_curr._state_kf, _trkpar_curr._covar_kf);
    if(!_extrapolator.extrapolateTo(z_pred))
    {
        return false;
    }

    _node.getPredicted()._z = z_pred;
    _extrapolator.getFinalStateWithCov(_node.getPredicted()._state_kf, _node.getPredicted()._covar_kf);
    _extrapolator.getPropagator(_node.getPropagator());
    _node.setPredictionDone();

    if(z_pred > recoConsts::instance()->get_DoubleFlag("FMAG_LENGTH"))
    {
        _node.getPredicted()._covar_kf = SMatrix::getABCt(_node.getPropagator(), _trkpar_curr._covar_kf, _node.getPropagator());
    }
    /*
    else
      {
        //double p_corr = ELOSS_CORR/FMAG_LENGTH*_extrapolator.getTravelLength()/10.;
        //_node.getPredicted()._state_kf[0][0] = _node.getPredicted().get_charge()/(_node.getPredicted().get_mom() - p_corr);
      }
    */

    ///Empty prediction, for debugging purpose
    //_node.getPredicted()._z = z_pred;
    //_node.getPredicted()._state_kf = _trkpar_curr._state_kf;
    //_node.getPredicted()._covar_kf = _trkpar_curr._covar_kf;
    //SMatrix::unitMatrix(_node.getPropagator());

    return true;
}

bool KalmanFilter::filter(Node& _node)
{
    if(_node.isFilterDone())
    {
        LogInfo("In filtering: Filter already done for this node!");
        return false;
    }

    if(!_node.isPredictionDone())
    {
        LogInfo("In filtering: Prediction not done for this node!");
        return false;
    }

    ///Get all the predicted state vector and covariance
    const TMatrixD& p_pred = _node.getPredicted()._state_kf;
    const TMatrixD& cov_pred = _node.getPredicted()._covar_kf;
    const TMatrixD& m = _node.getMeasurement();
    const TMatrixD& cov_m = _node.getMeasurementCov();
    const TMatrixD& proj = _node.getProjector();

    ///Calculate the filtered covariance matrix
    ///cov_filter = ( c_pred^{-1} + proj^t*G*proj)^{-1}
    TMatrixD g = SMatrix::invertMatrix(cov_m);
    TMatrixD cov_pred_inv = SMatrix::invertMatrix(cov_pred);
    TMatrixD cov_filter = SMatrix::invertMatrix(cov_pred_inv + SMatrix::getAtBC(proj, g, proj));

    ///Calculate the filtered state vector
    ///p_filter = c_filter.(c_pred^{-1}.p_pred + h^t.g.measure)
    ///Three equations below are mathematically equal, but EQ.2 appears to be faster
    //EQ. 1:
    //TMatrixD htgm = SMatrix::getAtBC(proj, g, m);
    //TMatrixD p_filter = cov_filter*(cov_pred_inv*p_pred + htgm);
    //EQ. 2:
    TMatrixD k = SMatrix::getABtC(cov_pred, proj, SMatrix::invertMatrix(cov_m + SMatrix::getABCt(proj, cov_pred, proj)));
    TMatrixD p_filter = p_pred + k*(m - proj*p_pred);
    //EQ. 3:
    //TMatrixD p_filter = p_pred + SMatrix::getABtC(cov_filter, proj, g)*(m - proj*p_pred);

    ///Calculate the filtered residule and associated covariance
    /// r_filter = m - h.p_filter
    /// r_cov_filter = m_cov - h.c_filter.h^t
    TMatrixD r_filter = m - proj*p_filter;
    TMatrixD r_cov_filter = cov_m - SMatrix::getABCt(proj, cov_filter, proj);

    ///Calculate the filtered parameter's contribution to chi square
    /// chi2 += chi2m + chi2p
    /// chi2m = r_filter^t.g.r_filter (contribution from measurement)
    /// chi2p = (p_filter-p_pred)^t.c_pred^{-1}.(p_filter - p_pred) (contribution from extrapolator)
    double chi2m = SMatrix::getAtBC(r_filter, g, r_filter)[0][0];
    TMatrixD dp = p_filter - p_pred;
    double chi2p = SMatrix::getAtBC(dp, cov_pred_inv, dp)[0][0];

    ///Store the filtered state vector
    _node.getFiltered()._state_kf = p_filter;
    _node.getFiltered()._covar_kf = cov_filter;
    _node.getFiltered()._z = _node.getPredicted()._z;
    _node.setChisq(chi2m + chi2p);
    _node.setFilterDone();

    /*
    if(_node.getFiltered().get_charge()*_node.getPredicted().get_charge() < 1)
      {
        LogInfo("ERROR!! Filter Wrong!");
        SMatrix::printMatrix(p_pred, "p_pred");
        SMatrix::printMatrix(p_filter, "p_filter");
        SMatrix::printMatrix(k, "k matrix");
        SMatrix::printMatrix(m - proj*p_pred, "m - proj*p_pred");
      }
    */

    return true;
}

bool KalmanFilter::smooth(Node& _node, Node& _node_prev)
{
    if(!_node.isFilterDone())
    {
        LogInfo("In smoother: Filter not done!");
        return false;
    }

    if(_node.isSmoothDone())
    {
        LogInfo("In smoother: Smooth already done!");
        return false;
    }

    if(!_node_prev.isSmoothDone())
    {
        LogInfo("In smoother: Smooth not done for the last node");
    }

    ///Retrieve related info
    const TMatrixD& p_filter = _node.getFiltered()._state_kf;
    const TMatrixD& cov_filter = _node.getFiltered()._covar_kf;
    const TMatrixD& p_pred_prev = _node_prev.getPredicted()._state_kf;
    const TMatrixD& cov_pred_prev = _node_prev.getPredicted()._covar_kf;
    const TMatrixD& p_smooth_prev = _node_prev.getSmoothed()._state_kf;
    const TMatrixD& cov_smooth_prev = _node_prev.getSmoothed()._covar_kf;
    const TMatrixD& prop_prev = _node_prev.getPropagator();

    ///Calculate smoothed state vector
    /// p_smooth = p_filter + a.(p_prev_smooth - p_prev_pred)
    /// a = c_filter.prop_prev^t.c_prev_pred^{-1}
    TMatrixD a = SMatrix::getABtCinv(cov_filter, prop_prev, cov_pred_prev);
    TMatrixD p_smooth = p_filter + a*(p_smooth_prev - p_pred_prev);

    ///Calculate the covariance of the smoothed state vector
    ///c_smooth = c_filter + a.(c_smooth_prev - c_pred_prev).a^t
    TMatrixD cov_smooth = cov_filter + SMatrix::getABCt(a, cov_smooth_prev - cov_pred_prev, a);

    ///Fill the smoothed track parameter
    _node.getSmoothed()._state_kf = p_smooth;
    _node.getSmoothed()._covar_kf = cov_smooth;
    _node.getSmoothed()._z = _node.getFiltered()._z;
    _node.setSmoothDone();

    return true;
}
