/*
SMillepedeUtil.cxx

Implementation of the class MPNode

Author: Kun Liu, liuk@fnal.gov
Created: 05-01-2013
*/

#include <iostream>

#include "GeomSvc.h"
#include "SMillepedeUtil.h"

ClassImp(MPNode)

MPNode::MPNode()
{
    detectorID = -1;
    flag = false;
}

MPNode::MPNode(int detector_index)
{
    detectorID = detector_index;
    flag = false;
}

MPNode::MPNode(Node& node_kalman)
{
    detectorID = node_kalman.getHit().detectorID;
    elementID = node_kalman.getHit().elementID;
    sign = node_kalman.getHit().driftDistance > 0 ? 1 : -1;

    charge = node_kalman.getFiltered().get_charge();
    tdctime = node_kalman.getHit().tdcTime;
    drift = node_kalman.getHit().driftDistance;
    pos = node_kalman.getHit().pos;

    if(detectorID >= 1 && detectorID <= nChamberPlanes)
    {
        flag = true;
    }
    else //Currently only the drift chambers are aligned
    {
        flag = false;
        return;
    }

    //Measurements
    meas = node_kalman.getSmoothedResidual()[0][0];
    sigma = sqrt(node_kalman.getSmoothedResidualCov()[0][0]);

    if(fabs(meas) > 1.5)
    {
        flag = false;
        return;
    }

    //Local parameters
    z = node_kalman.getZ() - Z_REF;
    tx = node_kalman.getSmoothed()._state_kf[1][0];
    ty = node_kalman.getSmoothed()._state_kf[2][0];
    x0 = node_kalman.getSmoothed()._state_kf[3][0] - z*tx;
    y0 = node_kalman.getSmoothed()._state_kf[4][0] - z*ty;

    //Plane tilt angles
    GeomSvc* p_geomSvc = GeomSvc::instance();
    double cosphi = p_geomSvc->getCostheta(detectorID);
    double sinphi = p_geomSvc->getSintheta(detectorID);

    //Fill direivatives
    setDerivatives(z, cosphi, sinphi, tx, ty, x0, y0);
}

MPNode::MPNode(SignedHit& hit_signed, Tracklet& trk)
{
    detectorID = hit_signed.hit.detectorID;
    elementID = hit_signed.hit.elementID;
    sign = hit_signed.sign;

    charge = trk.getCharge();
    tdctime = hit_signed.hit.tdcTime;
    drift = hit_signed.hit.driftDistance;
    pos = hit_signed.hit.pos;

    if(detectorID >= 1 && detectorID <= nChamberPlanes)
    {
        flag = true;
    }
    else //Currently only the drift chambers are aligned
    {
        flag = false;
        return;
    }

    //Measurements
    meas = trk.residual[detectorID-1];
    if(fabs(meas) > 1.5 )
    {
        flag = false;
        return;
    }

    //Local parameters
    GeomSvc* p_geomSvc = GeomSvc::instance();
    z = p_geomSvc->getPlanePosition(detectorID);
    tx = trk.tx;
    ty = trk.ty;
    x0 = trk.x0;
    y0 = trk.y0;
    if(trk.kmag_on && detectorID <= 12) trk.getXZInfoInSt1(tx, x0);

    //Plane tilt angles
    double cosphi = p_geomSvc->getCostheta(detectorID);
    double sinphi = p_geomSvc->getSintheta(detectorID);

    //double err_x = trk.getExpPosErrorX(z);
    //double err_y = trk.getExpPosErrorY(z);
    //sigma = sqrt(err_x*err_x*cosphi*cosphi + err_y*err_y*sinphi*sinphi);//fabs(err_x*cosphi) + fabs(err_y*sinphi);
    sigma = p_geomSvc->getPlaneResolution(detectorID);

    //Fill direivatives
    setDerivatives(z, cosphi, sinphi, tx, ty, x0, y0);
}

void MPNode::setDerivatives(double z, double cosphi, double sinphi, double tx, double ty, double x0, double y0)
{
    //Global derivatives
    dwdz = (tx*cosphi + ty*sinphi);
    dwdphi = (-x0*sinphi - tx*z*sinphi + y0*cosphi + ty*z*cosphi);
    dwdw = -1.;

    //Local derivatives
    dwdx = cosphi;
    dwdy = sinphi;
    dwdtx = z*cosphi;
    dwdty = z*sinphi;
}

void MPNode::print()
{
    using namespace std;

    cout << "========= Alignment node of detector: " << detectorID << " =========" << endl;
    if(!isValid())
    {
        cout << "No measurement on this node." << endl;
        return;
    }

    cout << "Measurements: " << meas << " +/- " << sigma << endl;

    cout << "Local track parameters: " << endl;
    cout << "z      x0     y0     dx/dz     dy/dz" << endl;
    cout << z << "    " << x0 << "    " << y0 << "    " << tx << "    " << ty << endl;

    cout << "Global derivative:" << endl;
    cout << "dw/dz       dw/dphi       dw/dw" << endl;
    cout << dwdz << "   " << dwdphi << "    " << dwdw << endl;

    cout << "Local derivative:" << endl;
    cout << "dw/dx     dw/dy      dw/dtx      dw/dty" << endl;
    cout << dwdx << "    " << dwdy << "    " << dwdtx << "    " << dwdty << endl;
}
