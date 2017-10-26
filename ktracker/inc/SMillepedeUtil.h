/*
SMillepedeUtil.h

Defines the supporting structures for millepede alignment, i.e. MPNode

Author: Kun Liu, liuk@fnal.gov
Created: Apr. 29, 2013
*/

#ifndef _SMILLEPEDEUTIL_H
#define _SMILLEPEDEUTIL_H

#include "GlobalConsts.h"

#include <TObject.h>
#include <TROOT.h>

#include "KalmanUtil.h"

class MPNode: public TObject
{
public:
    MPNode();
    MPNode(int detector_index);
    MPNode(Node& node_kalman);
    MPNode(SignedHit& hit_input, Tracklet& trk);
    //MPNode(MPNode& node_mp);

    //Calculate derivatives
    void setDerivatives(double z, double cosphi, double sinphi, double tx, double ty, double x0, double y0);

    //Flag indicating whether this node is valid or not
    bool flag;
    bool isValid() { return flag; }

    //detector ID
    int detectorID;
    int elementID;

    //Left/right
    int sign;

    //Measurement (actually residual due to the definition of millepede)
    double meas;

    //other auxilary info
    int charge;
    double tdctime;
    double pos;
    double drift;

    //Resolution of the measurement
    double sigma;

    //Derivatives w.r.t global parameters
    double dwdz;
    double dwdphi;
    double dwdw;

    //Derivarives w.r.t local parameters
    double dwdx;
    double dwdy;
    double dwdtx;
    double dwdty;

    //Local parameters
    double x0;            // x position
    double y0;            // y position
    double tx;            // dxdz, i.e. px/pz
    double ty;            // dydz, i.e. py/pz

    //z position of the node
    double z;

    //Overiden comparison operator
    bool operator<(const MPNode& elem) const { return detectorID < elem.detectorID; }

    //Debugging output
    void print();

    ClassDef(MPNode, 2)
};

#endif
