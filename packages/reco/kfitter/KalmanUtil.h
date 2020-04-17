/*
KalmanUtil.h

Utilities for kalman filter, including:
1. TrkPar: track parameter defination: (q/p, px, py, x, y), its covariance, z
2. Node: node defination for kalman filter
3. SMatrix: some frequently used matrix manipulations

Author: Kun Liu, liuk@fnal.gov
Created: 11-20-2011
*/

#ifndef _KALMANUTIL_H
#define _KALMANUTIL_H

#include "GlobalConsts.h"

#include <iostream>
#include <cmath>
#include <string>

#include <TMatrixD.h>
#include <TVector3.h>

#include "SRawEvent.h"
#include "FastTracklet.h"

class SMatrix
{
public:
    static void printMatrix(const TMatrixD& m);
    static void printMatrix(const TMatrixD& m, std::string str);
    static TMatrixD invertMatrix(const TMatrixD& m);
    static TMatrixD invertMatrixFast(const TMatrixD& m);
    static TMatrixD transposeMatrix(const TMatrixD& m);
    static void unitMatrix(TMatrixD& m);
    static void zeroMatrix(TMatrixD& m);
    static TMatrixD getABC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C);
    static TMatrixD getABCt(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C);
    static TMatrixD getAtBC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C);
    static TMatrixD getABtC(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C);
    static TMatrixD getABtCinv(const TMatrixD& A, const TMatrixD& B, const TMatrixD& C);
};

class TrkPar
{
public:
    TrkPar()
    {
        _state_kf.ResizeTo(5, 1);
        _covar_kf.ResizeTo(5, 5);
        _z = 0;
    }

    ///Gets
    const TMatrixD& get_state_vector() { return _state_kf; }
    const TMatrixD& get_covariance() {return _covar_kf; }
    double get_x() { return _state_kf(3, 0); }
    double get_y() { return _state_kf(4, 0); }
    double get_z() { return _z; }
    double get_dxdz() { return _state_kf(1, 0); }
    double get_dydz() { return _state_kf(2, 0); }
    double get_mom(double& px, double& py, double& pz);
    double get_mom() { return 1./fabs(_state_kf[0][0]); }
    TVector3 get_mom_vec() { double px, py, pz; get_mom(px, py, pz); return TVector3(px, py, pz); }
    double get_pos(double& x, double& y, double& z);
    int get_charge() { return _state_kf[0][0] > 0 ? 1 : -1; }

    ///Sets
    void set_state_vector(TMatrixD& state) { _state_kf = state; }
    void set_covariance(TMatrixD& cov) { _covar_kf = cov; }
    void set_x(double val) { _state_kf[3][0] = val; }
    void set_y(double val) { _state_kf[4][0] = val; }
    void set_z(double val) { _z = val; }
    void set_dxdz(double val) { _state_kf[1][0] = val; }
    void set_dydz(double val) { _state_kf[2][0] = val; }

    //Flip the sign of this track parameter
    void flip_charge();

    ///print for debugging purpose
    void print();

    ///State vectors and its covariance
    TMatrixD _state_kf;
    TMatrixD _covar_kf;
    double _z;
};

class Node
{
public:
    ///default constructor, only initialize the matrix dimension
    Node();

    ///constructor from a Hit to a Node
    Node(const Hit& _hit);

    ///Constructor from a signed hit to a Node
    Node(const SignedHit& _hit_signed);

    ///print for debugging purposes
    void print(bool verbose = true);

    ///Gets
    TrkPar& getPredicted() { return _predicted; }
    TrkPar& getFiltered() { return _filtered; }
    TrkPar& getSmoothed() { return _smoothed; }

    TMatrixD& getMeasurement() { return _measurement; }
    TMatrixD& getMeasurementCov() { return _measurement_cov; }

    ///Matrix calculations, should be called as less as possible
    TMatrixD getPredictedResidual();
    TMatrixD getPredictedResidualCov();
    TMatrixD getFilteredResidual();
    TMatrixD getFilteredResidualCov();
    TMatrixD getSmoothedResidual();
    TMatrixD getSmoothedResidualCov();

    TMatrixD& getPropagator() { return _propagator; }
    TMatrixD& getProjector() { return _projector; }

    TMatrixD getKalmanGain();

    double getZ() { return _z; }
    double getChisq() { return _chisq; }

    Hit& getHit() { return _hit; }

    bool isPredictionDone() { return _prediction_done; }
    bool isFilterDone() { return _filter_done; }
    bool isSmoothDone() { return _smooth_done; }

    ///Sets
    void setMeasurement(TMatrixD& m, TMatrixD& cov);
    void setZ(double z) { _z = z; }
    void setProjector(TMatrixD& p) { _projector = p; }
    void setPropagator(TMatrixD& p) { _propagator = p; };

    void setPredictionDone(bool flag = true) { _prediction_done = flag; }
    void setFilterDone(bool flag = true) { _filter_done = flag; }
    void setSmoothDone(bool flag = true) { _smooth_done = flag; }

    void setChisq(double chisq) { _chisq = chisq; }
    void addChisq(double chisq) { _chisq += chisq; }

    void resetFlags();

    ///Overriden operators
    bool operator<(const Node& elem) const { return _z < elem._z; };

private:
    TMatrixD _measurement;
    TMatrixD _measurement_cov;
    TMatrixD _projector;
    TMatrixD _propagator;

    double _z;

    bool _prediction_done;
    bool _filter_done;
    bool _smooth_done;

    TrkPar _predicted;
    TrkPar _filtered;
    TrkPar _smoothed;
    double _chisq;

    Hit _hit;
};

#endif
