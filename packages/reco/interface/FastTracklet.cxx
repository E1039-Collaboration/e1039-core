/*
FastTracklet.cxx

Implementation of class Tracklet

Author: Kun Liu, liuk@fnal.gov
Created: 05-28-2013
*/

#include <phool/recoConsts.h>

#include <iostream>
#include <algorithm>
#include <cmath>

#include <TMath.h>
#include <TMatrixD.h>

#include "FastTracklet.h"
#include "TriggerRoad.h"

ClassImp(SignedHit)
ClassImp(PropSegment)
ClassImp(Tracklet)
ClassImp(TrackletVector)

namespace 
{
    //static flag to indicate the initialized has been done
    static bool inited = false;

	//static pointer to geomtry service
	static GeomSvc* p_geomSvc = nullptr;

	//static flag of kmag on/off
	static bool KMAG_ON;

    //corase geometry
    static bool COARSE_MODE;

	//static flag of kmag strength
	static double FMAGSTR;
	static double KMAGSTR;

	static double PT_KICK_FMAG;
	static double PT_KICK_KMAG;

    //Track quality cuts
    static double TX_MAX;
    static double TY_MAX;
    static double X0_MAX;
    static double Y0_MAX;
    static double INVP_MAX;
    static double INVP_MIN;
    static double PROB_LOOSE;
    static double PROB_TIGHT;

    //Geometric positions
    static double Z_KMAG_BEND;
    static double Z_ABSORBER;
    static double Z_FMAG_BEND;
    static double Z_KFMAG_BEND;
    static double ELOSS_KFMAG;
    static double ELOSS_ABSORBER;

    //initialize global variables
    void initGlobalVariables()
    {
        if(!inited) 
        {
            inited = true;
            p_geomSvc = GeomSvc::instance();

            recoConsts* rc = recoConsts::instance();
            KMAG_ON = rc->get_BoolFlag("KMAG_ON");
            COARSE_MODE = rc->get_BoolFlag("COARSE_MODE");

            FMAGSTR = rc->get_DoubleFlag("FMAGSTR");
            KMAGSTR = rc->get_DoubleFlag("KMAGSTR");
            PT_KICK_FMAG = rc->get_DoubleFlag("PT_KICK_FMAG")*FMAGSTR;
            PT_KICK_KMAG = rc->get_DoubleFlag("PT_KICK_KMAG")*KMAGSTR;

            TX_MAX = rc->get_DoubleFlag("TX_MAX");
            TY_MAX = rc->get_DoubleFlag("TY_MAX");
            X0_MAX = rc->get_DoubleFlag("X0_MAX");
            Y0_MAX = rc->get_DoubleFlag("Y0_MAX");
            INVP_MAX = rc->get_DoubleFlag("INVP_MAX");
            INVP_MIN = rc->get_DoubleFlag("INVP_MIN");
            PROB_LOOSE = rc->get_DoubleFlag("PROB_LOOSE");
            PROB_TIGHT = rc->get_DoubleFlag("PROB_TIGHT");

            Z_KMAG_BEND = p_geomSvc->Z_KMAG_BEND();
            Z_ABSORBER = p_geomSvc->Z_ABSORBER();
            Z_FMAG_BEND = p_geomSvc->Z_FMAG_BEND();
            Z_KFMAG_BEND = p_geomSvc->Z_KFMAG_BEND();
            ELOSS_KFMAG = p_geomSvc->ELOSS_KFMAG();
            ELOSS_ABSORBER = p_geomSvc->ELOSS_ABSORBER();
        }
    }
}

//Signed hit definition
SignedHit::SignedHit() : sign(0)
{
}

SignedHit::SignedHit(int detectorID) : sign(0)
{
    hit.index = -1;
    hit.detectorID = detectorID;
}

SignedHit::SignedHit(Hit hit_input, int sign_input) : hit(hit_input), sign(sign_input)
{
}

void SignedHit::identify(std::ostream& os) const 
{
    if(sign > 0) os << "L - ";
    if(sign < 0) os << "R - ";
    if(sign == 0) os << "U - ";

    os << hit.index << " " << hit.detectorID << "  " << hit.elementID << std::endl;
}

//Proptube segment definition
//const GeomSvc* PropSegment::p_geomSvc = GeomSvc::instance();

PropSegment::PropSegment() : a(-999.), b(-999.), err_a(100.), err_b(100.), chisq(1.E6), nHodoHits(0)
{
    for(int i = 0; i < 4; ++i) hits[i].hit.index = -1;
    for(int i = 0; i < 4; ++i) hodoHits[i].index = -1;
    initGlobalVariables();
}

void PropSegment::init()
{
    a = -999.;
    b = -999.;
    err_a = 100;
    err_b = 100;

    for(int i = 0; i < 4; ++i) hits[i].hit.index = -1;

    chisq = 1E6;
}

void PropSegment::print(std::ostream& os) const
{
    using namespace std;

    os << "nHits: " << getNHits() << ", nPlanes: " << getNPlanes() << endl;
    os << "a = " << a << ", b = " << b << ", chisq = " << chisq << endl;
    os << "TX_MAX = " << TX_MAX << ", X0_MAX = " << X0_MAX << ", chisq max = " << 5 << endl;
    //os << "Absorber projection: " << getExpPosition(MUID_Z_REF) << endl;

    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].sign > 0) os << "L: ";
        if(hits[i].sign < 0) os << "R: ";
        if(hits[i].sign == 0) os << "U: ";

        os << hits[i].hit.index << "  " << hits[i].hit.detectorID << "  " << hits[i].hit.elementID << " === ";
    }
    os << endl;
}

double PropSegment::getClosestApproach(double z, double pos)
{
    return (a*z + b - pos)/sqrt(a*a + 1.);
}

double PropSegment::getPosRef(double default_val)
{
    if(hits[0].hit.index < 0 && hits[1].hit.index < 0) return default_val;

    int nRefPoints = 0;
    double pos_exp = 0.;
    for(int i = 0; i < 2; ++i)
    {
        if(hits[i].hit.index < 0) continue;

        pos_exp += hits[i].hit.pos;
        ++nRefPoints;
    }

    return pos_exp/nRefPoints;
}

int PropSegment::getNHits() const
{
    int nHits = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].hit.index >= 0) ++nHits;
    }
    return nHits;
}

int PropSegment::getNPlanes() const
{
    int nPlanes = 0;
    for(int i = 0; i < 2; ++i)
    {
        if(hits[i].hit.index >= 0 && hits[i+1].hit.index >= 0) ++nPlanes;
    }
    return nPlanes;
}

int PropSegment::isValid() const
{
    if(getNHits() < 2) return 0;
    if(getNPlanes() != 2) return 0;
    if(chisq > 5.) return 0;

    //May need optimization
    if(fabs(a) > TX_MAX) return 0;
    if(fabs(b) > X0_MAX) return 0;

    return 1;
}

void PropSegment::resolveLR()
{
    //Sign assignment for 1st and 2nd hits
    if(hits[0].hit.index > 0 && hits[1].hit.index > 0)
    {
        if(hits[0].hit.elementID == hits[1].hit.elementID)
        {
            hits[0].sign = -1;
            hits[1].sign = 1;
        }
        else
        {
            hits[0].sign = 1;
            hits[1].sign = -1;
        }
    }
    else
    {
        hits[0].sign = 0;
        hits[1].sign = 0;
    }

    //Sign assignment for 3rd and 4th hits
    if(hits[2].hit.index > 0 && hits[3].hit.index > 0)
    {
        if(hits[2].hit.elementID == hits[3].hit.elementID)
        {
            hits[2].sign = -1;
            hits[3].sign = 1;
        }
        else
        {
            hits[2].sign = 1;
            hits[3].sign = -1;
        }
    }
    else
    {
        hits[2].sign = 0;
        hits[3].sign = 0;
    }
}

void PropSegment::resolveLR(int settings)
{
    hits[0].sign = 2*(settings & 1) - 1;
    hits[1].sign = 2*((settings & 2) >> 1) - 1;
    hits[2].sign = 2*((settings & 4) >> 1) - 1;
    hits[3].sign = 2*((settings & 8) >> 1) - 1;
}

void PropSegment::fit()
{
    int nHits = getNHits();
    if(nHits == 2)
    {
        fit_2hits();
    }
    else
    {
        fit_34hits();
    }

    //remove one bad hits if possible/needed
    while(nHits > 2 && chisq > 5.)
    {
        int index = -1;
        double res_max = 0.;
        for(int i = 0; i < 4; ++i)
        {
            if(hits[i].hit.index < 0) continue;

            double res = fabs(hits[i].pos() - a*p_geomSvc->getPlanePosition(hits[i].hit.detectorID) - b);
            if(res > res_max)
            {
                index = i;
                res_max = res;
            }
        }

#ifdef _DEBUG_ON
        LogInfo("Invoking prop segment re-fitting...");
        LogInfo("Removing hit " << index << " with residual = " << res_max);
        LogInfo("Before removing a = " << a << ", b = " << b << ", chisq = " << chisq);
#endif

        //remove the bad hit
        hits[index].hit.index = -1;

        //fit again
        --nHits;
        if(nHits == 2)
        {
            fit_2hits();
        }
        else
        {
            fit_34hits();
        }

#ifdef _DEBUG_ON
        LogInfo("After removing a = " << a << ", b = " << b << ", chisq = " << chisq);
#endif
    }
}

void PropSegment::fit_2hits()
{
    double z[2], pos[2];

    int idx = 0;
    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].hit.index < 0) continue;

        z[idx] = p_geomSvc->getPlanePosition(hits[i].hit.detectorID);
        pos[idx] = hits[i].hit.pos;

        ++idx;
    }

    a = (pos[1] - pos[0])/(z[1] - z[0]);
    b = pos[0] - a*z[0];
    err_a = 1.5;
    err_b = 1.5;
    chisq = 0.;
}

/*
void PropSegment::fit_34hits()
{
  int index_min = -1;
  double chisq_min = 1E8;
  for(int i = 0; i < 16; ++i)
    {
      resolveLR(i);

      linearFit_iterative();
      if(chisq < chisq_min)
	{
	  chisq_min = chisq;
	  index_min = i;
	}
    }

  resolveLR(index_min);
  linearFit_iterative();
}
*/

void PropSegment::fit_34hits()
{
    resolveLR();
    linearFit_simple();
}

void PropSegment::linearFit_simple()
{
    double sum = 0.;
    double sx = 0.;
    double sy = 0.;
    double sxx = 0.;
    double syy = 0.;
    double sxy = 0.;

    double x[14], y[14];
    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].hit.index < 0) continue;

        y[i] = hits[i].pos();
        x[i] = p_geomSvc->getPlanePosition(hits[i].hit.detectorID);

        ++sum;
        sx += x[i];
        sy += y[i];
        sxx += (x[i]*x[i]);
        syy += (y[i]*y[i]);
        sxy += (x[i]*y[i]);
    }

    /*
    for(int i = 0; i < nHodoHits; ++i)
    {
        int idx = i + 4;
        y[idx] = hodoHits[i].pos;
        x[idx] = p_geomSvc->getPlanePosition(hodoHits[i].detectorID);

        ++sum;
        sx += x[idx];
        sy += y[idx];
        sxx += (x[idx]*x[idx]);
        syy += (y[idx]*y[idx]);
        sxy += (x[idx]*y[idx]);
    }
    */

    double det = sum*sxx - sx*sx;
    if(fabs(det) < 1E-20) return;

    a = (sum*sxy - sx*sy)/det;
    b = (sy*sxx - sxy*sx)/det;
    err_a = sqrt(fabs(sum/det));
    err_b = sqrt(fabs(sxx/det));

    chisq = 0.;
    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].hit.index < 0) continue;
        chisq += ((y[i] - a*x[i] -b)*(y[i] - a*x[i] -b));
    }
}

void PropSegment::linearFit_iterative()
{
    //Algorithm refer to Kun's PhD thesis

    //prepare the raw data
    int len = 0;
    double x[4], y[4], r[4];    //note r here is signed drift distance
    for(int i = 0; i < 4; ++i)
    {
        if(hits[i].hit.index < 0) continue;

        y[len] = hits[i].pos();
        x[len] = p_geomSvc->getPlanePosition(hits[i].hit.detectorID);
        r[len] = hits[i].sign*hits[i].hit.driftDistance;

        ++len;
    }

    //while loop with less than 100 iterations
    a = 0;
    int iter = 0;
    double a_prev = -999.;        // value of a in previous iteration
    double _x[4], _y[4];      // corrected hit pos
    while(fabs(a_prev - a) > 1E-4 && ++iter < 100)
    {
        a_prev = a;

        //prepare corrected hit pos
        double C, D, E, F, G, H;
        C = 0.;
        D = 0.;
        E = 0.;
        F = 0.;
        G = 0.;
        H = 0.;

        for(int i = 0; i < len; ++i)
        {
            _x[i] = x[i] - r[i]*a/sqrt(a*a + 1.);
            _y[i] = y[i] + r[i]/sqrt(a*a + 1.);

            C += 1.;
            D += _x[i];
            E += _y[i];
            F += _x[i]*_x[i];
            G += _y[i]*_y[i];
            H += _x[i]*_y[i];
        }

        double alpha = H - D*E/C;
        double beta = F - G -D*D/C + E*E/C;

        a = (-beta + sqrt(beta*beta + 4*alpha*alpha))/alpha/2.;
        b = (E - a*D)/C;

        chisq = 0.;
        for(int i = 0; i < len; ++i)
        {
            chisq += (a*_x[i] + b - _y[i])*(a*_x[i] + b - _y[i])/(a*a + 1.);
        }
    }
}


Tracklet::Tracklet() : stationID(-1), nXHits(0), nUHits(0), nVHits(0), chisq(9999.), chisq_vtx(9999.), tx(0.), ty(0.), x0(0.), y0(0.), invP(0.1), err_tx(-1.), err_ty(-1.), err_x0(-1.), err_y0(-1.), err_invP(-1.)
{
    for(int i = 0; i < nChamberPlanes; i++) residual[i] = 999.;
    initGlobalVariables();
}

int Tracklet::isValid() const
{
    if(stationID < 1 || stationID > nStations) return 0;
    if(fabs(tx) > TX_MAX || fabs(x0) > X0_MAX) return 0;
    if(fabs(ty) > TY_MAX || fabs(y0) > Y0_MAX) return 0;
    if(err_tx < 0 || err_ty < 0 || err_x0 < 0 || err_y0 < 0) return 0;

    double prob = getProb();
    if(stationID != nStations && prob < PROB_LOOSE) return 0;
    
    //Tracklets in each station
    int nHits = nXHits + nUHits + nVHits;
    if(stationID < nStations-1)
    {
        if(nXHits < 1 || nUHits < 1 || nVHits < 1) return 0;
        if(nHits < 4) return 0;
        if(chisq > 40.) return 0;
    }
    else
    {
        //Number of hits cuts, second index is X, U, V, first index is station-1, 2, 3
        int nRealHits[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
        nXHits = 0; nUHits = 0; nVHits = 0;
        for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
        {
            if(iter->hit.index < 0) continue;

            int idx1 = iter->hit.detectorID <= 12 ? 0 : (iter->hit.detectorID <= 18 ? 1 : 2);
            int idx2 = p_geomSvc->getPlaneType(iter->hit.detectorID) - 1;

            ++nRealHits[idx1][idx2];
            if(idx2 == 0)
                ++nXHits;
            else if(idx2 == 1)
                ++nUHits;
            else
                ++nVHits;
        }

        //Number of hits cut after removing bad hits
        for(int i = 1; i < 3; ++i)
        {
            if(nRealHits[i][0] < 1 || nRealHits[i][1] < 1 || nRealHits[i][2] < 1) return 0;
            if(nRealHits[i][0] + nRealHits[i][1] + nRealHits[i][2] < 4) return 0;
        }

        //for global tracks only -- TODO: may need to set a new station-1 cut
        if(stationID == nStations)
        {
            if(nRealHits[0][0] < 1 || nRealHits[0][1] < 1 || nRealHits[0][2] < 1) return 0;
            if(nRealHits[0][0] + nRealHits[0][1] + nRealHits[0][2] < 4) return 0;

            if(prob < PROB_TIGHT) return 0;
            if(KMAG_ON)
            {
                if(invP < INVP_MIN || invP > INVP_MAX) return 0;
            }
        }
    }

    return 1;
}

double Tracklet::getProb() const
{
    int ndf;
    if(stationID == nStations && KMAG_ON)
    {
        ndf = getNHits() - 5;
    }
    else
    {
        ndf = getNHits() - 4;
    }

    return TMath::Prob(chisq, ndf);
    //return -chisq/ndf;
}

double Tracklet::getMomProb() const
{
    double weights[40] = {0, 0, 3.1292e-05, 0.00203877, 0.0147764, 0.0417885, 0.08536, 0.152212, 0.250242, 0.322011, 0.327125, 0.275443, 0.220316, 0.189932, 0.162112, 0.131674, 0.100102, 0.0736696, 0.0510353, 0.0364215, 0.0233914, 0.0152907, 0.00992716, 0.00601322, 0.00382757, 0.00239005, 0.00137169, 0.000768309, 0.000413311, 0.00019659, 8.31216e-05, 2.77721e-05, 7.93826e-06, 7.45884e-07, 2.57648e-08, 0, 6.00088e-09, 0, 0, 0};
    int index = int(1./invP/2.5);

    return (index >= 0 && index < 40) ? (weights[index] < 1.E-5 ? 1.E-5 : weights[index]) : 1.E-5;
}

TVector3 Tracklet::getExpMomentum(double z) const
{
    return (KMAG_ON && stationID >= nStations-1 && z < Z_KMAG_BEND - 1.) ? getMomentumSt1() : getMomentumSt3();
}

double Tracklet::getExpPositionX(double z) const
{
    if(KMAG_ON && stationID >= nStations-1 && z < Z_KMAG_BEND - 1.)
    {
        double tx_st1 = tx + PT_KICK_KMAG*invP*getCharge();
        double x0_st1 = tx*Z_KMAG_BEND + x0 - tx_st1*Z_KMAG_BEND;

        return x0_st1 + tx_st1*z;
    }
    else
    {
        return x0 + tx*z;
    }
}

double Tracklet::getExpPosErrorX(double z) const
{
    double err_x;
    if(KMAG_ON && stationID >= nStations-1 && z < Z_KMAG_BEND - 1.)
    {
        double err_kick = fabs(err_invP*PT_KICK_KMAG);
        double err_tx_st1 = err_tx + err_kick;
        double err_x0_st1 = err_x0 + err_kick*Z_KMAG_BEND;

        err_x = err_x0_st1 + fabs(err_tx_st1*z);
    }
    else
    {
        err_x = fabs(err_tx*z) + err_x0;
    }

    if(z > Z_ABSORBER) err_x += 1.;
    return err_x;
}

double Tracklet::getExpPositionY(double z) const
{
    return y0 + ty*z;
}

double Tracklet::getExpPosErrorY(double z) const
{
    double err_y = fabs(err_ty*z) + err_y0;
    if(z > Z_ABSORBER) err_y += 1.;

    return err_y;
}

double Tracklet::getExpPositionW(int detectorID) const
{
    double z = p_geomSvc->getPlanePosition(detectorID);

    double x_exp = getExpPositionX(z);
    double y_exp = getExpPositionY(z);

    if(!p_geomSvc->isInPlane(detectorID, x_exp, y_exp)) return 999999.;
    return p_geomSvc->getCostheta(detectorID)*x_exp + p_geomSvc->getSintheta(detectorID)*y_exp;
}

int Tracklet::getExpElementID(int detectorID) const
{
  return p_geomSvc->getExpElementID(detectorID, getExpPositionW(detectorID));
}

bool Tracklet::operator<(const Tracklet& elem) const
{
    //return nXHits + nUHits + nVHits - 0.4*chisq > elem.nXHits + elem.nUHits + elem.nVHits - 0.4*elem.chisq;
    if(getNHits() == elem.getNHits())
    {
        return chisq < elem.chisq;
    }
    else
    {
        return getProb() > elem.getProb();
    }
}

bool Tracklet::similarity(const Tracklet& elem) const
{
    int nCommonHits = 0;
    std::list<SignedHit>::const_iterator first = hits.begin();
    std::list<SignedHit>::const_iterator second = elem.hits.begin();

    while(first != hits.end() && second != elem.hits.end())
    {
        if((*first) < (*second))
        {
            ++first;
        }
        else if((*second) < (*first))
        {
            ++second;
        }
        else
        {
            if((*first) == (*second)) nCommonHits++;
            ++first;
            ++second;
        }
    }

    if(nCommonHits/double(elem.getNHits()) > 0.33333) return true;
    return false;
}

double Tracklet::getMomentum() const
{
    //Ref. SEAQUEST-doc-453-v3 by Don. Geesaman
    //if(KMAG_ON == 0) return 1E8;

    double p = 50.;
    double charge = getCharge();

    double c1 = Z_FMAG_BEND*PT_KICK_FMAG*charge;
    double c2 = Z_KMAG_BEND*PT_KICK_KMAG*charge;
    double c3 = -x0;
    double c4 = ELOSS_KFMAG/2.;
    double c5 = ELOSS_KFMAG;

    double b = c1/c3 + c2/c3 - c4 - c5;
    double c = c4*c5 - c1*c5/c3 - c2*c4/c3;

    double disc = b*b - 4*c;
    if(disc > 0.)
    {
        p = (-b + sqrt(disc))/2. - ELOSS_KFMAG;
    }

    if(p < 10. || p > 120. || disc < 0)
    {
        double k = fabs(getExpPositionX(Z_KFMAG_BEND)/Z_KFMAG_BEND - tx);
        p = 1./(0.00832161 + 0.184186*k - 0.104132*k*k) + ELOSS_ABSORBER;
    }

    return p;
}

/// Return the charge (+1 or -1) of this tracklet.
/**
 * This function should be as simple as possible, in order to reduce the
 * computation time.  Therefore the condition of the charge determination
 * uses only "x0" and "tx" (at St. 2+3).  The formula was obtained 
 * practically by the study in DocDB 9505.  This function is valid for both 
 * the parallel and anti-parallel FMag+KMag polarity combination.  But it 
 * is _not_ guaranteed to be valid when the FMag and/or KMag field strength
 * is changed largely.
 */
int Tracklet::getCharge() const
{
    return -3e-3 * copysign(1.0, FMAGSTR) * x0 < tx  ?  +1  :  -1;
}

void Tracklet::getXZInfoInSt1(double& tx_st1, double& x0_st1) const
{
    if(KMAG_ON)
    {
        tx_st1 = tx + PT_KICK_KMAG*invP*getCharge();
        x0_st1 = tx*Z_KMAG_BEND + x0 - tx_st1*Z_KMAG_BEND;
    }
    else
    {
        tx_st1 = tx;
        x0_st1 = x0;
    }
}

void Tracklet::getXZErrorInSt1(double& err_tx_st1, double& err_x0_st1) const
{
    if(KMAG_ON)
    {
        double err_kick = fabs(err_invP*PT_KICK_KMAG);
        err_tx_st1 = err_tx + err_kick;
        err_x0_st1 = err_x0 + err_kick*Z_KMAG_BEND;
    }
    else
    {
        err_tx_st1 = err_tx;
        err_x0_st1 = err_x0;
    }
}

Tracklet Tracklet::operator+(const Tracklet& elem) const
{
    Tracklet tracklet;
    tracklet.stationID = nStations - 1;

    tracklet.nXHits = nXHits + elem.nXHits;
    tracklet.nUHits = nUHits + elem.nUHits;
    tracklet.nVHits = nVHits + elem.nVHits;

    tracklet.hits.assign(hits.begin(), hits.end());
    if(elem.stationID > stationID)
    {
        tracklet.hits.insert(tracklet.hits.end(), elem.hits.begin(), elem.hits.end());
    }
    else
    {
        tracklet.hits.insert(tracklet.hits.begin(), elem.hits.begin(), elem.hits.end());
    }

    tracklet.err_tx = 1./sqrt(1./err_tx/err_tx + 1./elem.err_tx/elem.err_tx);
    tracklet.err_ty = 1./sqrt(1./err_ty/err_ty + 1./elem.err_ty/elem.err_ty);
    tracklet.err_x0 = 1./sqrt(1./err_x0/err_x0 + 1./elem.err_x0/elem.err_x0);
    tracklet.err_y0 = 1./sqrt(1./err_y0/err_y0 + 1./elem.err_y0/elem.err_y0);

    tracklet.tx = (tx/err_tx/err_tx + elem.tx/elem.err_tx/elem.err_tx)*tracklet.err_tx*tracklet.err_tx;
    tracklet.ty = (ty/err_ty/err_ty + elem.ty/elem.err_ty/elem.err_ty)*tracklet.err_ty*tracklet.err_ty;
    tracklet.x0 = (x0/err_x0/err_x0 + elem.x0/elem.err_x0/elem.err_x0)*tracklet.err_x0*tracklet.err_x0;
    tracklet.y0 = (y0/err_y0/err_y0 + elem.y0/elem.err_y0/elem.err_y0)*tracklet.err_y0*tracklet.err_y0;

    tracklet.invP = 1./tracklet.getMomentum();
    tracklet.err_invP = 0.25*tracklet.invP;

    tracklet.calcChisq();
    return tracklet;
}

Tracklet Tracklet::operator*(const Tracklet& elem) const
{
    Tracklet tracklet;
    tracklet.stationID = nStations;

    tracklet.nXHits = nXHits + elem.nXHits;
    tracklet.nUHits = nUHits + elem.nUHits;
    tracklet.nVHits = nVHits + elem.nVHits;

    tracklet.hits.assign(hits.begin(), hits.end());
    if(elem.stationID > stationID)
    {
        tracklet.hits.insert(tracklet.hits.end(), elem.hits.begin(), elem.hits.end());
    }
    else
    {
        tracklet.hits.insert(tracklet.hits.begin(), elem.hits.begin(), elem.hits.end());
    }

    if(elem.stationID == nStations - 1)
    {
        tracklet.tx = elem.tx;
        tracklet.ty = elem.ty;
        tracklet.x0 = elem.x0;
        tracklet.y0 = elem.y0;
        tracklet.invP = 1./elem.getMomentum();

        tracklet.err_tx = elem.err_tx;
        tracklet.err_ty = elem.err_ty;
        tracklet.err_x0 = elem.err_x0;
        tracklet.err_y0 = elem.err_y0;
        tracklet.err_invP = 0.25*tracklet.invP;
    }
    else
    {
        tracklet.tx = tx;
        tracklet.ty = ty;
        tracklet.x0 = x0;
        tracklet.y0 = y0;
        tracklet.invP = 1./getMomentum();

        tracklet.err_tx = err_tx;
        tracklet.err_ty = err_ty;
        tracklet.err_x0 = err_x0;
        tracklet.err_y0 = err_y0;
        tracklet.err_invP = 0.25*tracklet.invP;
    }

    tracklet.calcChisq();
    return tracklet;
}

Tracklet Tracklet::merge(Tracklet& elem)
{
    Tracklet tracklet;
    tracklet.stationID = stationID;

    tracklet.tx = tx;
    tracklet.ty = ty;
    tracklet.x0 = x0;
    tracklet.y0 = y0;
    tracklet.invP = 1./getMomentum();

    tracklet.err_tx = err_tx;
    tracklet.err_ty = err_ty;
    tracklet.err_x0 = err_x0;
    tracklet.err_y0 = err_y0;
    tracklet.err_invP = 0.25*tracklet.invP;

    tracklet.chisq_vtx = chisq_vtx < 999 ? chisq_vtx : elem.chisq_vtx;

    tracklet.seg_x = seg_x;
    tracklet.seg_y = seg_y;

    std::list<SignedHit>::iterator elemend = elem.hits.begin();
    for(int i = 0; i < 6; ++i) ++elemend;
    tracklet.hits.insert(tracklet.hits.begin(), hits.begin(), hits.end());
    tracklet.hits.insert(tracklet.hits.begin(), elem.hits.begin(), elemend);
    tracklet.hits.sort();

    tracklet.calcChisq();
    tracklet.isValid();
    return tracklet;
}

void Tracklet::addDummyHits()
{
    std::vector<int> detectorIDs_all;
    for(int i = stationID*6 - 5; i <= stationID*6; ++i) detectorIDs_all.push_back(i);

    std::vector<int> detectorIDs_now;
    for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
        detectorIDs_now.push_back(iter->hit.detectorID);
    }

    std::vector<int> detectorIDs_miss(6);
    std::vector<int>::iterator iter = std::set_difference(detectorIDs_all.begin(), detectorIDs_all.end(), detectorIDs_now.begin(), detectorIDs_now.end(), detectorIDs_miss.begin());
    detectorIDs_miss.resize(iter - detectorIDs_miss.begin());

    for(std::vector<int>::iterator iter = detectorIDs_miss.begin(); iter != detectorIDs_miss.end(); ++iter)
    {
        SignedHit dummy;
        dummy.hit.detectorID = *iter;

        hits.push_back(dummy);
    }

    sortHits();
}

/**
 * The drift distance of a hit is ignored if the drift sign of the hit is not fixed.
 */
double Tracklet::calcChisq()
{
    chisq = 0.;

    double tx_st1, x0_st1;
    if(stationID == nStations && KMAG_ON)
    {
        getXZInfoInSt1(tx_st1, x0_st1);
    }

    for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;

        int detectorID = iter->hit.detectorID;
        int index = detectorID - 1;

        double sigma;
        if(iter->sign == 0 || COARSE_MODE) 
            sigma = p_geomSvc->getPlaneSpacing(detectorID)/sqrt(12.);
            //sigma = fabs(iter->hit.driftDistance)/sqrt(12.);
        else
            sigma = p_geomSvc->getPlaneResolution(detectorID);

        //double p = iter->hit.pos + iter->sign*fabs(iter->hit.driftDistance);
        if(KMAG_ON && stationID == nStations && detectorID <= 12)
        {
            //residual[index] = p - p_geomSvc->getInterception(detectorID, tx_st1, ty, x0_st1, y0);
            residual[index] = iter->sign*fabs(iter->hit.driftDistance) - p_geomSvc->getDCA(detectorID, iter->hit.elementID, tx_st1, ty, x0_st1, y0);
        }
        else
        {
            //residual[index] = p - p_geomSvc->getInterception(detectorID, tx, ty, x0, y0);
            residual[index] = iter->sign*fabs(iter->hit.driftDistance) - p_geomSvc->getDCA(detectorID, iter->hit.elementID, tx, ty, x0, y0);
        }

        chisq += (residual[index]*residual[index]/sigma/sigma);
        //std::cout << iter->hit.detectorID << "  " << iter->hit.elementID << "  " << iter->sign << "  " << iter->hit.pos << "  " << iter->hit.driftDistance << "  " << residual[index] << "  " <<  sigma << "  " << chisq << std::endl;
    }

    //std::cout << chisq << std::endl;
    return chisq;
}

SignedHit Tracklet::getSignedHit(int index)
{
    int id = 0;
    for(std::list<SignedHit>::const_iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
        if(id == index) return *iter;
        id++;
    }

    SignedHit dummy;
    return dummy;
}

double Tracklet::Eval(const double* par)
{
    tx = par[0];
    ty = par[1];
    x0 = par[2];
    y0 = par[3];
    if(KMAG_ON) invP = par[4];

    //std::cout << tx << "  " << ty << "  " << x0 << "  " << y0 << "  " << 1./invP << std::endl;
    return calcChisq();
}

double Tracklet::Eval4(const double* par)
{
    tx = par[0];
    ty = par[1];
    x0 = par[2];
    y0 = par[3];

    //std::cout << tx << "  " << ty << "  " << x0 << "  " << y0 << "  " << 1./invP << std::endl;
    return calcChisq();
}

SRecTrack Tracklet::getSRecTrack(bool hyptest)
{
    SRecTrack strack;
    strack.setChisq(chisq);
    for(std::list<SignedHit>::iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
        if(iter->hit.index < 0) continue;

        double z = p_geomSvc->getPlanePosition(iter->hit.detectorID);
        double tx_val, tx_err, x0_val, x0_err;
        if(iter->hit.detectorID <= 12)
        {
            getXZInfoInSt1(tx_val, x0_val);
            getXZErrorInSt1(tx_err, x0_err);
        }
        else
        {
            tx_val = tx;
            x0_val = x0;
            tx_err = err_tx;
            x0_err = err_x0;
        }

        TMatrixD state(5, 1), covar(5, 5);
        state[0][0] = getCharge()*invP*sqrt((1. + tx_val*tx_val)/(1. + tx_val*tx_val + ty*ty));
        state[1][0] = tx_val;
        state[2][0] = ty;
        state[3][0] = getExpPositionX(z);
        state[4][0] = getExpPositionY(z);

        covar.Zero();
        covar[0][0] = err_invP*err_invP;
        covar[1][1] = tx_err*tx_err;
        covar[2][2] = err_ty*err_ty;
        covar[3][3] = getExpPosErrorX(z)*getExpPosErrorX(z);
        covar[4][4] = getExpPosErrorY(z)*getExpPosErrorY(z);

        strack.insertHitIndex(iter->hit.index*iter->sign);
        strack.insertStateVector(state);
        strack.insertCovariance(covar);
        strack.insertZ(z);
    }

    //Set single vertex swimming
    strack.swimToVertex(nullptr, nullptr, hyptest);

    //Set trigger road info
    TriggerRoad road(*this);
    strack.setTriggerRoad(road.getRoadID());

    //Set prop tube slopes
    strack.setNHitsInPT(seg_x.getNHits(), seg_y.getNHits());
    strack.setPTSlope(seg_x.a, seg_y.a);

    return strack;
}

TVector3 Tracklet::getMomentumSt1() const
{
    double tx_st1, x0_st1;
    getXZInfoInSt1(tx_st1, x0_st1);

    double pz = 1./invP/sqrt(1. + tx_st1*tx_st1);
    return TVector3(pz*tx_st1, pz*ty, pz);
}

TVector3 Tracklet::getMomentumSt3() const
{
    double pz = 1./invP/sqrt(1. + tx*tx);
    return TVector3(pz*tx, pz*ty, pz);
}

void Tracklet::print(std::ostream& os)
{
    using namespace std;

    calcChisq();
    TVector3 mom1 = getMomentumSt1();
    TVector3 mom3 = getMomentumSt3();

    os << "Tracklet in station " << stationID << endl;
    os << nXHits + nUHits + nVHits << " hits in this station with chisq = " << chisq << endl;
    os << "Momentum in z @st1: " << mom1.Pz() << " +/- " << mom1.Pz()*err_invP/invP << endl;
    os << "Momentum in z @st3: " << mom3.Pz() << " +/- " << mom3.Pz()*err_invP/invP << endl;
    os << "Charge: " << getCharge() << endl;
    for(std::list<SignedHit>::iterator iter = hits.begin(); iter != hits.end(); ++iter)
    {
        if(iter->sign > 0) os << "L: ";
        if(iter->sign < 0) os << "R: ";
        if(iter->sign == 0) os << "U: ";

        os << iter->hit.index << " " << p_geomSvc->getDetectorName(iter->hit.detectorID) << "(" << iter->hit.detectorID << ") " << iter->hit.elementID << "  " << residual[iter->hit.detectorID-1] << " = ";
    }
    os << endl;

    os << "X-Z: (" << tx << " +/- " << err_tx << ")*z + (" << x0 << " +/- " << err_x0 << ")" << endl;
    os << "Y-Z: (" << ty << " +/- " << err_ty << ")*z + (" << y0 << " +/- " << err_y0 << ")" << endl;

    os << "KMAG projection: X =  " << getExpPositionX(Z_KMAG_BEND) << " +/- " << getExpPosErrorX(Z_KMAG_BEND) << endl;
    os << "KMAG projection: Y =  " << getExpPositionY(Z_KMAG_BEND) << " +/- " << getExpPosErrorY(Z_KMAG_BEND) << endl;
    os << "KMAGSTR =  " << KMAGSTR << endl;
}

TrackletVector::TrackletVector(): trackletVec()
{}

TrackletVector::~TrackletVector()
{
    Reset();
}

void TrackletVector::identify(std::ostream& os) const
{
    os << "TrackletVector with " << size() << " entries" << std::endl;
}

void TrackletVector::Reset()
{
    for(auto iter = trackletVec.begin(); iter != trackletVec.end(); ++iter) delete (*iter);
    trackletVec.clear();
}

const Tracklet* TrackletVector::at(const size_t index) const
{
    if(index >= size()) return nullptr;
    return trackletVec[index];
}

Tracklet* TrackletVector::at(const size_t index)
{
    if(index >= size()) return nullptr;
    return trackletVec[index];
}

void TrackletVector::push_back(const Tracklet* tracklet)
{
    trackletVec.push_back(tracklet->Clone());
}

size_t TrackletVector::erase(const size_t index)
{
    if(index >= size()) return size();

    delete trackletVec[index];
    trackletVec.erase(trackletVec.begin() + index);
    return trackletVec.size();
}
