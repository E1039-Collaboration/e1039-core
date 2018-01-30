#ifndef _KALMANFITTER_H
#define _KALMANFITTER_H

/*
KalmanFitter.h

Definition of the class KalmanFitter, refine the measurement of KalmanTrack

Author: Kun Liu, liuk@fnal.gov
Created: 10-18-2012
*/

#include <list>
#include <vector>

#include "GlobalConsts.h"

#include "KalmanUtil.h"
#include "KalmanTrack.h"
#include "KalmanFilter.h"

class KalmanFitter
{
public:
    KalmanFitter();
    //~KalmanFitter();

    ///Set the convergence control parameters
    void setControlParameter(int nMaxIteration, double tolerance) { _max_iteration = nMaxIteration; _tolerance = tolerance; }

    ///external call to process one single tracks
    ///the prediction-filter-smooth cycle is iteratively done
    ///until the chi square converges
    int processOneTrack(KalmanTrack& _track);
    void updateTrack(KalmanTrack& _track);

    ///Initialize the kalman filter
    void init();

    ///Initialize the node list
    int initNodeList(KalmanTrack& _track);

    ///Update the actual z position of each node according to current fit results
    void updateAlignment();

    ///Set the starting parameters
    void setStartingParameter(KalmanTrack& _track);
    void setStartingParameter(Node& _node);

    ///Initialize the smoother
    bool initSmoother(Node& _node);

    ///return the node list
    std::list<Node>& getNodeList() { return _nodes; }

    ///Vertex finder
    double findVertex();

    ///Get the final results -- temporary interfaces, just for debugging purposes
    double getMomentumInitial(double& px, double& py, double& pz) { return _nodes.front().getSmoothed().get_mom(px, py, pz); }
    double getMomentumFinal(double& px, double& py, double& pz) { return _nodes.back().getSmoothed().get_mom(px, py, pz); }

    double getPositionInitial(double& x, double& y, double& z) { return _nodes.front().getSmoothed().get_pos(x, y, z); }
    double getPositionFinal(double& x, double& y, double& z) { return _nodes.back().getSmoothed().get_pos(x, y, z); }

    double getChisq() { return _chisq; }

    const TrkPar& getTrkParInitial() { return _nodes.front().getSmoothed(); }
    const TrkPar& getTrkParFinal() { return _nodes.back().getSmoothed(); }

    //TrkPar getTrkPar(double z) { std::cout << "Will be implemented later" << std::endl; }

private:
    ///list of all nodes associated with this track
    std::list<Node> _nodes;

    ///Chi square for the current fit
    double _chisq;

    ///Pointer to an instance of Kalman filter
    KalmanFilter *_kmfit;

    ///cache of the rotation matrix of all detector planes
    double rM_20[nChamberPlanes];
    double rM_21[nChamberPlanes];
    double rM_22[nChamberPlanes];
    double z_planes[nChamberPlanes];

    ///Control variables
    int _max_iteration;
    double _tolerance;
};

#endif
