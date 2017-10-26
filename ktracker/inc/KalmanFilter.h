/*
KalmanFilter.h

Definition of the light-weighted version of class KalmanFilter, provide only the core function
of the kalman filter, includes:
1. prediction
2. filter
3. smooth

Author: Kun Liu, liuk@fnal.gov
Created: 10-14-2011
*/

#ifndef _KALMANFILTER_H
#define _KALMANFILTER_H

#include "GlobalConsts.h"

#include <list>
#include <vector>

#include "GeomSvc.h"
#include "KalmanUtil.h"
#include "TrackExtrapolator.hh"

class KalmanFilter
{
public:
    ///singlton instance
    static KalmanFilter* instance();
    void close();

    ///Real constructor
    KalmanFilter(bool limitedStep = true);

    ///Kalman filter steps
    bool predict(Node& _node);
    bool filter(Node& _node);
    bool smooth(Node& _node, Node& _node_prev);

    ///set the current track parameter using the current node
    void setCurrTrkpar(Node& _node) { _trkpar_curr = _node.getFiltered(); }
    void setCurrTrkpar(TrkPar& _trkpar) { _trkpar_curr = _trkpar; }

    ///Fit one node
    bool fit_node(Node& _node);

    ///Enable the dump mode: stop calc prop matrix, start calc travel length
    void enableDumpCorrection() { _extrapolator.setPropCalc(true); _extrapolator.setLengthCalc(true); }

private:
    ///Pointer to singlton instance
    static KalmanFilter *p_kmfit;

    ///Stores the current track parameter
    TrkPar _trkpar_curr;

    ///Track extrapolator
    TrackExtrapolator _extrapolator;
};

#endif
