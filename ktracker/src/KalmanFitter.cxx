/*
kalmanFitter.cxx

Implimentation of the class KalmanFitter, provides all core
functionalities of the track fitting procedure

Author: Kun Liu, liuk@fnal.gov
Created: 10-18-2012
*/

#include <iostream>
#include <algorithm>
#include <cmath>

#include "GeomSvc.h"
#include "KalmanFitter.h"

KalmanFitter::KalmanFitter()
{
    _kmfit = KalmanFilter::instance();

    _max_iteration = 100;
    _tolerance = 1E-3;

    GeomSvc* p_geomSvc = GeomSvc::instance();
    for(int i = 1; i <= nChamberPlanes; ++i)
    {
        double rX = p_geomSvc->getRotationInX(i);
        double rY = p_geomSvc->getRotationInY(i);
        double rZ = p_geomSvc->getRotationInZ(i);

        z_planes[i-1] = p_geomSvc->getPlaneCenterZ(i);
        rM_20[i-1] = cos(rX)*sin(rY)*cos(rZ) + sin(rX)*sin(rZ);
        rM_21[i-1] = cos(rX)*sin(rY)*sin(rZ) - sin(rX)*cos(rZ);
        rM_22[i-1] = cos(rY)*cos(rX);;
    }
}

void KalmanFitter::init()
{
    _chisq = 0.;
    for(std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); ++iter)
    {
        iter->resetFlags();
    }
}

int KalmanFitter::processOneTrack(KalmanTrack& _track)
{
    //LogInfo("Start processing this track..");
    int iIter = 0;

    ///Initialize the nodes from the input track
    initNodeList(_track);

    ///Set the initial track parameters
    setStartingParameter(_track);
    updateAlignment();

    ///Call the kalman filter core functions iteratively until the chi square converges
    double _chisq_curr = 1E6;
    for(; iIter < _max_iteration; iIter++)
    {
        //LogInfo("Iteration " << iIter << " starts: ");
        init();

        for(std::list<Node>::reverse_iterator node = _nodes.rbegin(); node != _nodes.rend(); ++node)
        {
            //LogInfo(node->getZ());
            if(_kmfit->fit_node(*node))
            {
                _kmfit->setCurrTrkpar(node->getFiltered());
                _chisq += node->getChisq();
            }
            else
            {
                LogInfo("Failed in prediction and filtering. ");
                return 0;
            }
        }

        std::list<Node>::iterator node = _nodes.begin();
        std::list<Node>::iterator last_smoothed = _nodes.begin();
        initSmoother(*node);
        ++node;
        for(; node != _nodes.end(); ++node)
        {
            if(!_kmfit->smooth(*node, *last_smoothed))
            {
                LogInfo("Failed in smoothing.");
                return 0;
            }
            last_smoothed = node;
        }

        /*
        for(std::list<Node>::reverse_iterator node = _nodes.rbegin(); node != _nodes.rend(); ++node)
        {
          node->print(true);
        }
        */

        //update the chisq to see if it converges
        //LogInfo("For this iteration, chisq = " << _chisq << ", chisq_curr = " << _chisq_curr);

        if(_chisq_curr < 0.) return -1;
        if(fabs(_chisq_curr - _chisq) < _tolerance) break;
        if(_chisq - _chisq_curr > 5.) break;

        _chisq_curr = _chisq;

        //update the starting parameters
        setStartingParameter(*last_smoothed);
        updateAlignment();
    }

    return iIter + 1;
}

void KalmanFitter::updateTrack(KalmanTrack& _track)
{
    _track.getNodeList().assign(_nodes.begin(), _nodes.end());
    _track.setCurrTrkpar(_nodes.front().getFiltered());
    _track.update();
}

int KalmanFitter::initNodeList(KalmanTrack& _track)
{
    _nodes.clear();

    _nodes.assign(_track.getNodeList().begin(), _track.getNodeList().end());
    _nodes.sort();

    return _nodes.size();
}

void KalmanFitter::updateAlignment()
{
    //update the z positon of each node according to the current fit
    // of (x, y)

    for(std::list<Node>::iterator node = _nodes.begin(); node != _nodes.end(); ++node)
    {
        double x, y;
        if(node->isSmoothDone())
        {
            x = node->getSmoothed().get_x();
            y = node->getSmoothed().get_y();
        }
        else if(node->isFilterDone())
        {
            x = node->getFiltered().get_x();
            y = node->getFiltered().get_y();
        }
        else
        {
            x = node->getPredicted().get_x();
            y = node->getPredicted().get_y();
        }

        int index = node->getHit().detectorID - 1;
        node->setZ(rM_20[index]*x + rM_21[index]*y + rM_22[index]*z_planes[index]);
    }
}

void KalmanFitter::setStartingParameter(Node& _node)
{
    TrkPar _trkpar_curr;

    ///Method 1.
    _trkpar_curr = _node.getSmoothed();

    /*
    ///Method 2.
    _trkpar_curr._state_kf = _node.getSmoothed()._state_kf;
    _trkpar_curr._covar_kf = ;
    _trkpar_curr._z = _node.getSmoothed()._z;
    */

    _kmfit->setCurrTrkpar(_trkpar_curr);
}


void KalmanFitter::setStartingParameter(KalmanTrack& _track)
{
    _kmfit->setCurrTrkpar(_track.getNodeList().back().getPredicted());
}

bool KalmanFitter::initSmoother(Node& _node)
{
    if(!_node.isFilterDone())
    {
        LogInfo("In smoother init: Filter not done!");
        return false;
    }

    if(_node.isSmoothDone())
    {
        LogInfo("In smoother init: Smooth already done!");
        return false;
    }

    _node.getSmoothed() = _node.getFiltered();
    _node.setSmoothDone();

    return true;
}
