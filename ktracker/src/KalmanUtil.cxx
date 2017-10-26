/*
KalmanUtil.cxx

Implementation of the utilities for KalmanFilter, some of the implementation
is already included in KalmanUtil.h

Author: Kun Liu, liuk@fnal.gov
Created: 11-20-2011
*/

#include <iostream>
#include <cmath>

#include "SRawEvent.h"
#include "KalmanUtil.h"

void SMatrix::printMatrix(const TMatrixD& m)
{
    int nRow = m.GetNrows();
    int nCol = m.GetNcols();

    std::cout << "The matrix has " << nRow << " rows and " << nCol << " columns." << std::endl;
    for(int i = 0; i < nRow; i++)
    {
        std::cout << "Line " << i << ":  ";
        for(int j = 0; j < nCol; j++)
        {
            std::cout << m[i][j] << "  ";
        }
        std::cout << std::endl;
    }
}

void SMatrix::printMatrix(const TMatrixD& m, std::string str)
{
    int nRow = m.GetNrows();
    int nCol = m.GetNcols();

    std::cout << "Printing the content of matrix: " << str << std::endl;
    std::cout << "The matrix has " << nRow << " rows and " << nCol << " columns." << std::endl;
    for(int i = 0; i < nRow; i++)
    {
        std::cout << "Line " << i << ":  ";
        for(int j = 0; j < nCol; j++)
        {
            std::cout << m[i][j] << "  ";
        }
        std::cout << std::endl;
    }
}

TMatrixD SMatrix::invertMatrix(const TMatrixD& m)
{
    TMatrixD mout = m;
    if(m.GetNrows() > 1 && m.GetNrows() <= 5)
    {
        mout.InvertFast();
    }
    else if(m.GetNrows() == 1)
    {
        mout[0][0] = 1./m[0][0];
    }
    else
    {
        mout.Invert();
    }

    return mout;
}

TMatrixD SMatrix::invertMatrixFast(const TMatrixD& m)
{
    TMatrixD mout = m;
    mout.InvertFast();

    return mout;
}

TMatrixD SMatrix::transposeMatrix(const TMatrixD& m)
{
    TMatrixD mout = m;
    mout.T();

    return mout;
}

void SMatrix::unitMatrix(TMatrixD& m)
{
    int nRows = m.GetNrows();
    int nCols = m.GetNcols();
    for(int i = 0; i < nRows; i++)
    {
        for(int j = 0; j < nCols; j++)
        {
            if(i == j)
            {
                m[i][j] = 1.;
            }
            else
            {
                m[i][j] = 0.;
            }
        }
    }
}

void SMatrix::zeroMatrix(TMatrixD& m)
{
    int nRows = m.GetNrows();
    int nCols = m.GetNcols();
    for(int i = 0; i < nRows; i++)
    {
        for(int j = 0; j < nCols; j++)
        {
            m[i][j] = 0.;
        }
    }
}

TMatrixD SMatrix::getABC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C)
{
    return A*B*C;
}

TMatrixD SMatrix::getABCt(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C)
{
    TMatrixD Ct = C;
    Ct.T();

    return A*B*Ct;
}

TMatrixD SMatrix::getAtBC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C)
{
    TMatrixD At = A;
    At.T();

    return At*B*C;
}

TMatrixD SMatrix::getABtC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C)
{
    TMatrixD Bt = B;
    Bt.T();

    return A*Bt*C;
}

TMatrixD SMatrix::getABtCinv(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C)
{
    TMatrixD Bt = B;
    Bt.T();
    TMatrixD Cinv = invertMatrix(C);

    return A*Bt*Cinv;
}

void TrkPar::flip_charge()
{
    _state_kf[0][0] = -1.*_state_kf[0][0];

    for(int i = 1; i < 5; i++)
    {
        _covar_kf[0][i] = -1.*_covar_kf[0][i];
        _covar_kf[i][0] = -1.*_covar_kf[i][0];
    }
}

double TrkPar::get_mom(double& px, double& py, double& pz)
{
    double p = 1./fabs(_state_kf[0][0]);
    pz = p/sqrt(1. + _state_kf[1][0]*_state_kf[1][0] + _state_kf[2][0]*_state_kf[2][0]);
    px = pz*_state_kf[1][0];
    py = pz*_state_kf[2][0];

    return p;
}

double TrkPar::get_pos(double& x, double& y, double& z)
{
    z = _z;
    x = _state_kf[3][0];
    y = _state_kf[4][0];

    return sqrt(x*x + y*y);
}

void TrkPar::print()
{
    std::cout << "Content of the track parameters at z = " << _z << std::endl;
    std::cout << "==============================================" << std::endl;
    SMatrix::printMatrix(_state_kf, "State vector");
    SMatrix::printMatrix(_covar_kf, "Covariance matrix");
    std::cout << "==============================================" << std::endl;
}

void Node::print(bool verbose)
{
    std::cout << "The status of this node " << std::endl;
    if(_prediction_done)
    {
        std::cout << "The prediction is done! " << std::endl;
        _predicted.print();
    }

    if(_filter_done)
    {
        std::cout << "The filtering is done! " << std::endl;
        _filtered.print();

        //SMatrix::printMatrix(getKalmanGain(), "Kalman gain matrix");
        SMatrix::printMatrix(_measurement, "The actual measurement");
        SMatrix::printMatrix(_projector*_predicted._state_kf, "The predicted measurement");
        SMatrix::printMatrix(_projector*_filtered._state_kf, "The filtered measurement");
    }

    if(_smooth_done)
    {
        std::cout << "The smoothing is done! " << std::endl;
        _smoothed.print();

        SMatrix::printMatrix(_projector*_smoothed._state_kf, "The smoothed measurement");
    }

    std::cout << "Chi square of this node: " << _chisq << std::endl;

    if(!verbose) return;
    SMatrix::printMatrix(_measurement, "The measurement of this node: ");
    SMatrix::printMatrix(_measurement_cov, "The covariance of the measurement of this node: ");
    SMatrix::printMatrix(_projector, "The projection matrix: ");
    SMatrix::printMatrix(_propagator, "The propagation matrix: ");
}

Node::Node()
{
    _measurement.ResizeTo(1, 1);
    _measurement_cov.ResizeTo(1, 1);

    _projector.ResizeTo(1, 5);
    _propagator.ResizeTo(5, 5);

    _prediction_done = false;
    _filter_done = false;
    _smooth_done = false;

    _chisq = 0.;
}

Node::Node(const Hit& hit_input)
{
    _hit = hit_input;

    _measurement.ResizeTo(1, 1);
    _measurement_cov.ResizeTo(1, 1);

    _projector.ResizeTo(1, 5);
    _propagator.ResizeTo(5, 5);

    _prediction_done = false;
    _filter_done = false;
    _smooth_done = false;

    _chisq = 0.;

    GeomSvc* geometrySvc = GeomSvc::instance();

    double sigma;
    sigma = geometrySvc->getPlaneResolution(_hit.detectorID);
    _measurement[0][0] = _hit.pos + _hit.driftDistance;
    _measurement_cov[0][0] = sigma*sigma;

    _projector[0][3] = geometrySvc->getCostheta(_hit.detectorID);
    _projector[0][4] = geometrySvc->getSintheta(_hit.detectorID);

    _z = geometrySvc->getPlanePosition(_hit.detectorID);
    _predicted._z = _z;
    _filtered._z = _z;
    _smoothed._z = _z;
}

Node::Node(const SignedHit& hit_input)
{
    _hit = hit_input.hit;
    _hit.index = _hit.index*hit_input.sign;

    _measurement.ResizeTo(1, 1);
    _measurement_cov.ResizeTo(1, 1);

    _projector.ResizeTo(1, 5);
    _propagator.ResizeTo(5, 5);

    _prediction_done = false;
    _filter_done = false;
    _smooth_done = false;

    _chisq = 0.;

    GeomSvc* geometrySvc = GeomSvc::instance();

    double sigma;
    if(hit_input.sign != 0)
    {
        sigma = geometrySvc->getPlaneResolution(_hit.detectorID);
    }
    else
    {
        sigma = _hit.driftDistance;//geometrySvc->getPlaneSpacing(_hit.detectorID)/sqrt(12.);
    }
    _measurement[0][0] = _hit.pos + hit_input.sign*_hit.driftDistance;
    _measurement_cov[0][0] = sigma*sigma;

    _projector[0][3] = geometrySvc->getCostheta(_hit.detectorID);
    _projector[0][4] = geometrySvc->getSintheta(_hit.detectorID);

    _z = geometrySvc->getPlanePosition(_hit.detectorID);
    _predicted._z = _z;
    _filtered._z = _z;
    _smoothed._z = _z;
}

void Node::setMeasurement(TMatrixD& m, TMatrixD& cov)
{
    _measurement = m;
    _measurement_cov = cov;
}

TMatrixD Node::getPredictedResidual()
{
    TMatrixD mout = _measurement;
    mout -= (_projector*_predicted._state_kf);

    return mout;
}

TMatrixD Node::getPredictedResidualCov()
{
    TMatrixD mout = _measurement_cov;
    mout += SMatrix::getABCt(_projector, _predicted._covar_kf, _projector);

    return mout;
}

TMatrixD Node::getFilteredResidual()
{
    TMatrixD mout = _measurement;
    mout -= (_projector*_filtered._state_kf);

    return mout;
}

TMatrixD Node::getFilteredResidualCov()
{
    TMatrixD mout = _measurement_cov;
    mout += SMatrix::getABCt(_projector, _filtered._covar_kf, _projector);

    return mout;
}

TMatrixD Node::getSmoothedResidual()
{
    TMatrixD mout = _measurement;
    mout -= (_projector*_smoothed._state_kf);

    return mout;
}

TMatrixD Node::getSmoothedResidualCov()
{
    TMatrixD mout = _measurement_cov;
    mout += SMatrix::getABCt(_projector, _smoothed._covar_kf, _projector);

    return mout;
}

TMatrixD Node::getKalmanGain()
{
    TMatrixD K = SMatrix::getABtC(_predicted._covar_kf, _projector, SMatrix::invertMatrix(_measurement_cov + SMatrix::getABCt(_projector, _predicted._covar_kf, _projector)));
    return K;
}

void Node::resetFlags()
{
    _prediction_done = false;
    _filter_done = false;
    _smooth_done = false;

    _chisq = 0.;
}

