/*
SRecEvent.cxx

Implimentation of the calss SRecTrack and SRecEvent

Author: Kun Liu, liuk@fnal.gov
Created: 01-21-2013
*/

#include <cmath>

#include <TLorentzVector.h>
#include <TMatrixD.h>

#include <GenFit/SharedPlanePtr.h>
#include <phool/recoConsts.h>

#include "SRecEvent.h"
#include "KalmanUtil.h"
#include "KalmanFilter.h"

ClassImp(SRecTrack)
ClassImp(SRecDimuon)
ClassImp(SRecEvent)

namespace 
{
    //static flag to indicate the initialized has been done
    static bool inited = false;

	//static flag of kmag on/off
	static bool KMAG_ON;

	//static flag of kmag strength
	static double FMAGSTR;
	static double KMAGSTR;

	static double PT_KICK_FMAG;
	static double PT_KICK_KMAG;

    //Geometric positions
    static double FMAG_LENGTH;
    static double Z_TARGET;
    static double Z_DUMP;
    static double Z_UPSTREAM;
    static double Z_DOWNSTREAM;
    static double FMAG_HOLE_LENGTH;
    static double FMAG_HOLE_RADIUS;

    //Beam position and shape
    static double X_BEAM;
    static double Y_BEAM;
    static double SIGX_BEAM;
    static double SIGY_BEAM;

    //Simple swimming settings 
    static int NSTEPS_TARGET;
    static int NSTEPS_SHIELDING;
    static int NSTEPS_FMAG;

    static double DEDX_UNIT_0;
    static double DEDX_UNIT_1;
    static double DEDX_UNIT_2;
    static double DEDX_UNIT_3;
    static double DEDX_UNIT_4;
    static double PTKICK_UNIT;

    static double STEP_TARGET;
    static double STEP_SHIELDING;
    static double STEP_FMAG;

    //initialize global variables
    void initGlobalVariables()
    {
        if(!inited) 
        {
            inited = true;

            recoConsts* rc = recoConsts::instance();
            KMAG_ON = rc->get_BoolFlag("KMAG_ON");
            FMAGSTR = rc->get_DoubleFlag("FMAGSTR");
            KMAGSTR = rc->get_DoubleFlag("KMAGSTR");
            PT_KICK_FMAG = rc->get_DoubleFlag("PT_KICK_FMAG")*FMAGSTR;
            PT_KICK_KMAG = rc->get_DoubleFlag("PT_KICK_KMAG")*KMAGSTR;
            
            X_BEAM = rc->get_DoubleFlag("X_BEAM");
            Y_BEAM = rc->get_DoubleFlag("Y_BEAM");
            SIGX_BEAM = rc->get_DoubleFlag("SIGX_BEAM");
            SIGY_BEAM = rc->get_DoubleFlag("SIGY_BEAM");

            FMAG_LENGTH = rc->get_DoubleFlag("FMAG_LENGTH");
            Z_TARGET = rc->get_DoubleFlag("Z_TARGET");
            Z_DUMP = rc->get_DoubleFlag("Z_DUMP");
            Z_UPSTREAM = rc->get_DoubleFlag("Z_UPSTREAM");
            Z_DOWNSTREAM = rc->get_DoubleFlag("Z_DOWNSTREAM");
            FMAG_HOLE_RADIUS = rc->get_DoubleFlag("FMAG_HOLE_RADIUS");
            FMAG_HOLE_LENGTH = rc->get_DoubleFlag("FMAG_HOLE_LENGTH");

            NSTEPS_TARGET = rc->get_IntFlag("NSTEPS_TARGET");
            NSTEPS_SHIELDING = rc->get_IntFlag("NSTEPS_SHIELDING");
            NSTEPS_FMAG = rc->get_IntFlag("NSTEPS_FMAG");

            DEDX_UNIT_0 = rc->get_DoubleFlag("DEDX_FE_P0")/FMAG_LENGTH;
            DEDX_UNIT_1 = rc->get_DoubleFlag("DEDX_FE_P1")/FMAG_LENGTH;
            DEDX_UNIT_2 = rc->get_DoubleFlag("DEDX_FE_P2")/FMAG_LENGTH;
            DEDX_UNIT_3 = rc->get_DoubleFlag("DEDX_FE_P3")/FMAG_LENGTH;
            DEDX_UNIT_4 = rc->get_DoubleFlag("DEDX_FE_P4")/FMAG_LENGTH;
            PTKICK_UNIT = PT_KICK_FMAG/FMAG_LENGTH;

            STEP_TARGET = fabs(Z_UPSTREAM)/NSTEPS_TARGET;
            STEP_SHIELDING = 0.;
            STEP_FMAG = FMAG_LENGTH/NSTEPS_FMAG/2.;
        }
    }
}

SRecTrack::SRecTrack()
{
    fChisq = -99.;

    fHitIndex.clear();
    fState.clear();
    fCovar.clear();
    fZ.clear();
    fChisqAtNode.clear();

    for(Int_t i = 0; i < 3; ++i) fGFDetPlaneVec[i].clear();
    fGFAuxInfo.clear();
    fGFStateVec.clear();
    fGFCov.clear();

    fChisqVertex = -99.;
    fVertexPos.SetXYZ(999., 999., 999.);
    fStateVertex.ResizeTo(5, 1);
    fCovarVertex.ResizeTo(5, 5);

    fChisqTarget = -99.;
    fChisqDump = -99.;
    fChisqUpstream = -99.;

    initGlobalVariables();
}

bool SRecTrack::operator<(const SRecTrack& elem) const
{
    return getNHits() == elem.getNHits() ? (fChisq < elem.fChisq) : (getProb() > elem.getProb());
}

Int_t SRecTrack::getNHitsInStation(Int_t stationID)
{
    if(stationID != 1 && stationID != 2 && stationID != 3) return 0;

    Double_t z_ref[4] = {0., 700., 1600., 2500.};
    Int_t nHits = 0;
    for(std::vector<Double_t>::iterator iter = fZ.begin(); iter != fZ.end(); ++iter)
    {
        if(*iter > z_ref[stationID-1] && *iter < z_ref[stationID]) ++nHits;
    }

    return nHits;
}

Double_t SRecTrack::getProb() const
{ 
    return KMAG_ON == 1 ? TMath::Prob(fChisq, getNHits() - 5) : TMath::Prob(fChisq, getNHits() - 4); 
}

void SRecTrack::setZVertex(Double_t z, bool update)
{
    Node _node_vertex;
    _node_vertex.setZ(z);

    TMatrixD m(2, 1), cov(2, 2),  proj(2, 5);
    m[0][0] = X_BEAM;
    m[1][0] = Y_BEAM;

    cov.Zero();
    cov[0][0] = SIGX_BEAM*SIGX_BEAM;
    cov[1][1] = SIGY_BEAM*SIGY_BEAM;

    proj.Zero();
    proj[0][3] = 1.;
    proj[1][4] = 1.;

    _node_vertex.getMeasurement().ResizeTo(2, 1);
    _node_vertex.getMeasurementCov().ResizeTo(2, 2);
    _node_vertex.getProjector().ResizeTo(2, 5);

    _node_vertex.getMeasurement() = m;
    _node_vertex.getMeasurementCov() = cov;
    _node_vertex.getProjector() = proj;

    TrkPar _trkpar_curr;
    _trkpar_curr._state_kf = fState[0];
    _trkpar_curr._covar_kf = fCovar[0];
    _trkpar_curr._z = fZ[0];

    KalmanFilter* kmfit = KalmanFilter::instance();
    kmfit->enableDumpCorrection();
    kmfit->setCurrTrkpar(_trkpar_curr);
    kmfit->fit_node(_node_vertex);

    fChisqVertex = _node_vertex.getChisq();
    if(!update) return;

    fVertexPos.SetXYZ(_node_vertex.getFiltered().get_x(), _node_vertex.getFiltered().get_y(), z);
    fVertexMom = _node_vertex.getFiltered().get_mom_vec();

    fStateVertex = _node_vertex.getFiltered()._state_kf;
    fCovarVertex = _node_vertex.getFiltered()._covar_kf;
}

void SRecTrack::updateVtxHypothesis()
{
    setZVertex(Z_TARGET, false);
    setChisqTarget(fChisqVertex);

    setZVertex(Z_DUMP, false);
    setChisqDump(fChisqVertex);

    setZVertex(Z_UPSTREAM+10., false);
    setChisqUpstream(fChisqVertex);

    setZVertex(getZVertex(), true);
}

void SRecTrack::setVertexFast(TVector3 mom, TVector3 pos)
{
    fVertexPos = pos;
    fVertexMom = mom;

    fStateVertex[0][0] = getCharge()/mom.Mag();
    fStateVertex[1][0] = mom[0]/mom[2];
    fStateVertex[2][0] = mom[1]/mom[2];
    fStateVertex[3][0] = pos[0];
    fStateVertex[4][0] = pos[1];

    fCovarVertex.UnitMatrix();
}

bool SRecTrack::isVertexValid() const
{
    if(fChisqVertex > 50.) return false;
    if(fVertexPos.Z() < Z_UPSTREAM || fVertexPos.Z() > Z_DOWNSTREAM) return false;

    return true;
}

Int_t SRecTrack::getNearestNode(Double_t z)
{
    Int_t nNodes = getNHits();
    Int_t index_min = 0;
    Double_t dist_min = 1E6;
    for(Int_t i = 0; i < nNodes; ++i)
    {
        Double_t dist = fabs(z - fZ[i]);
        if(dist < dist_min)
        {
            dist_min = dist;
            index_min = i;
        }
    }

    return index_min;
}

void SRecTrack::getExpPositionFast(Double_t z, Double_t& x, Double_t& y, Int_t iNode)
{
    if(iNode < 0 || iNode >= getNHits())
    {
        iNode = getNearestNode(z);
    }

    TrkPar _trkpar;
    _trkpar._state_kf = fState[iNode];
    _trkpar._covar_kf = fCovar[iNode];

    Double_t z_ref = fZ[iNode];
    Double_t x_ref = _trkpar.get_x();
    Double_t y_ref = _trkpar.get_y();
    Double_t axz = _trkpar.get_dxdz();
    Double_t ayz = _trkpar.get_dydz();

    x = x_ref + axz*(z - z_ref);
    y = y_ref + ayz*(z - z_ref);
}

void SRecTrack::getExpPosErrorFast(Double_t z, Double_t& dx, Double_t& dy, Int_t iNode)
{
    if(iNode < 0 || iNode >= getNHits())
    {
        iNode = getNearestNode(z);
    }

    Double_t z_ref = fZ[iNode];
    Double_t dx_ref = sqrt((fCovar[iNode])[3][3]);
    Double_t dy_ref = sqrt((fCovar[iNode])[4][4]);
    Double_t daxz = sqrt((fCovar[iNode])[1][1]);
    Double_t dayz = sqrt((fCovar[iNode])[2][2]);

    dx = 2.*(dx_ref + fabs(daxz*(z - z_ref)));
    dy = 2.*(dy_ref + fabs(dayz*(z - z_ref)));
}

double SRecTrack::getExpMomentumFast(Double_t z, Int_t iNode)
{
    Double_t px, py, pz;
    return getExpMomentumFast(z, px, py, pz, iNode);
}

Double_t SRecTrack::getExpMomentumFast(Double_t z, Double_t& px, Double_t& py, Double_t& pz, Int_t iNode)
{
    if(iNode < 0 || iNode >= getNHits())
    {
        iNode = getNearestNode(z);
    }

    return getMomentum(fState[iNode], px, py, pz);

}

Double_t SRecTrack::getMomentum(const TMatrixD& state, Double_t& px, Double_t& py, Double_t& pz) const
{
    Double_t p = 1./fabs(state[0][0]);
    pz = p/sqrt(1. + state[1][0]*state[1][0] + state[2][0]*state[2][0]);
    px = pz*state[1][0];
    py = pz*state[2][0];

    return p;
}

Double_t SRecTrack::getPosition(const TMatrixD& state, Double_t& x, Double_t& y) const
{
    x = state[3][0];
    y = state[4][0];

    return sqrt(x*x + y*y);
}

TLorentzVector SRecTrack::getMomentumVertex()
{
    Double_t mmu = 0.10566;
    Double_t px, py, pz, E;

    getMomentumVertex(px, py, pz);
    E = sqrt(px*px + py*py + pz*pz + mmu*mmu);

    return TLorentzVector(px, py, pz, E);
}

void SRecTrack::insertGFState(const genfit::MeasuredStateOnPlane& msop)
{
    fGFStateVec.push_back(msop.getState());
    fGFCov.push_back(msop.getCov());
    fGFAuxInfo.push_back(msop.getAuxInfo());

    const genfit::SharedPlanePtr& plane = msop.getPlane();
    fGFDetPlaneVec[0].push_back(plane->getO());
    fGFDetPlaneVec[1].push_back(plane->getU());
    fGFDetPlaneVec[2].push_back(plane->getV());
}

void SRecTrack::adjustKMag(double kmagStr)
{
    for(std::vector<TMatrixD>::iterator iter = fState.begin(); iter != fState.end(); ++iter)
    {
        (*iter)[0][0] = (*iter)[0][0]/kmagStr;
    }
}

int SRecTrack::isValid() const
{
    //Vertex valid
    if(!isVertexValid()) return false;

    //Number of hits cut
    Int_t nHits = getNHits();
    if(nHits < 14) return false;

    //Trigger road cut
    //if(fTriggerID == 0) return false;

    //Total chisq, may change to cut on prob
    if(getChisq()/(nHits - 5) > 15.) return false;

    //Check the px polarity
    //if(FMAGSTR*getCharge()*fVertexMom.Px() < 0) return false;

    return true;
}

bool SRecTrack::isTarget()
{
    return (fVertexPos.Z() > -300 && fVertexPos.Z() < 0. && fChisqDump - fChisqTarget > 10.);
}

bool SRecTrack::isDump()
{
    return (fVertexPos.Z() > 0. && fVertexPos.Z() < 150. && fChisqTarget - fChisqDump > 10.);
}

void SRecTrack::swimToVertex(TVector3* pos, TVector3* mom, bool hyptest)
{
    //Store the steps on each point (center of the interval)
    bool cleanupPos = false;
    bool cleanupMom = false;
    if(pos == nullptr)
    {
        pos = new TVector3[NSTEPS_FMAG + NSTEPS_TARGET + 1];
        cleanupPos = true;
    }
    if(mom == nullptr)
    {
        mom = new TVector3[NSTEPS_FMAG + NSTEPS_TARGET + 1];
        cleanupMom = true;
    }

    //track slope/location in upstream
    double tx = fState.front()[1][0];
    double ty = fState.front()[2][0];
    double x0 = fState.front()[3][0];
    double y0 = fState.front()[4][0];
    double z0 = fZ.front();

    //Initial position should be on the downstream face of beam dump
    pos[0].SetXYZ(x0 + tx*(FMAG_LENGTH - z0), y0 + ty*(FMAG_LENGTH - z0), FMAG_LENGTH);
    mom[0] = getMomentumVecSt1();

    //Charge of the track
    double charge = getCharge();

    //Now make the swim
    int iStep = 1;
    for(; iStep <= NSTEPS_FMAG; ++iStep)
    {
        //Make pT kick at the center of slice, add energy loss at both first and last half-slice
        //Note that ty is the global class data member, which does not change during the entire swimming
        double tx_i = mom[iStep-1].Px()/mom[iStep-1].Pz();
        double tx_f = tx_i + 2.*charge*PTKICK_UNIT*STEP_FMAG/sqrt(mom[iStep-1].Px()*mom[iStep-1].Px() + mom[iStep-1].Pz()*mom[iStep-1].Pz());

        TVector3 trajVec1(tx_i*STEP_FMAG, ty*STEP_FMAG, STEP_FMAG);
        TVector3 pos_b = pos[iStep-1] - trajVec1;

        double p_tot_i = mom[iStep-1].Mag();
        double p_tot_b;
        if(pos_b[2] > FMAG_HOLE_LENGTH || pos_b.Perp() > FMAG_HOLE_RADIUS)
        {
            p_tot_b = p_tot_i + (DEDX_UNIT_0 + p_tot_i*DEDX_UNIT_1 + p_tot_i*p_tot_i*DEDX_UNIT_2 + p_tot_i*p_tot_i*p_tot_i*DEDX_UNIT_3 + p_tot_i*p_tot_i*p_tot_i*p_tot_i*DEDX_UNIT_4)*trajVec1.Mag();
        }
        else
        {
            p_tot_b = p_tot_i;
        }

        TVector3 trajVec2(tx_f*STEP_FMAG, ty*STEP_FMAG, STEP_FMAG);
        pos[iStep] = pos_b - trajVec2;

        double p_tot_f;
        if(pos[iStep][2] > FMAG_HOLE_LENGTH || pos[iStep].Perp() > FMAG_HOLE_RADIUS)
        {
            p_tot_f = p_tot_b + (DEDX_UNIT_0 + p_tot_b*DEDX_UNIT_1 + p_tot_b*p_tot_b*DEDX_UNIT_2 + p_tot_b*p_tot_b*p_tot_b*DEDX_UNIT_3 + p_tot_b*p_tot_b*p_tot_b*p_tot_b*DEDX_UNIT_4)*trajVec2.Mag();
        }
        else
        {
            p_tot_f = p_tot_b;
        }

        //Now the final position and momentum in this step
        double pz_f = p_tot_f/sqrt(1. + tx_f*tx_f + ty*ty);
        mom[iStep].SetXYZ(pz_f*tx_f, pz_f*ty, pz_f);
        pos[iStep] = pos[iStep-1] - trajVec1 - trajVec2;

        //Save the dump position when applicable
        if(fabs(pos_b.Z() - Z_DUMP) < STEP_FMAG)
        {
            double dz = Z_DUMP - pos_b.Z();
            if(dz < 0)
            {
                setDumpPos(pos_b + TVector3(tx_f*dz, ty*dz, dz));
                setDumpMom(mom[iStep]);
            }
            else
            {
                setDumpPos(pos_b + TVector3(tx_i*dz, ty*dz, dz));
                setDumpMom(mom[iStep-1]);
            }
        }

#ifdef _DEBUG_ON_LEVEL_2
        std::cout << "FMAG: " << iStep << ": " << pos[iStep-1][2] << " ==================>>> " << pos[iStep][2] << std::endl;
        std::cout << mom[iStep-1][0]/mom[iStep-1][2] << "     " << mom[iStep-1][1]/mom[iStep-1][2] << "     " << mom[iStep-1][2] << "     ";
        std::cout << pos[iStep-1][0] << "  " << pos[iStep-1][1] << "   " << pos[iStep-1][2] << std::endl << std::endl;
        std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
        std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif
    }

    for(; iStep < NSTEPS_FMAG+NSTEPS_TARGET+1; ++iStep)
    {
        //Simple straight line flight
        double tx_i = mom[iStep-1].Px()/mom[iStep-1].Pz();
        TVector3 trajVec(tx_i*STEP_TARGET, ty*STEP_TARGET, STEP_TARGET);

        mom[iStep] = mom[iStep-1];
        pos[iStep] = pos[iStep-1] - trajVec;

#ifdef _DEBUG_ON_LEVEL_2
        std::cout << "TARGET: " << iStep << ": " << pos[iStep-1][2] << " ==================>>> " << pos[iStep][2] << std::endl;
        std::cout << mom[iStep-1][0]/mom[iStep-1][2] << "     " << mom[iStep-1][1]/mom[iStep-1][2] << "     " << mom[iStep-1][2] << "     ";
        std::cout << pos[iStep-1][0] << "  " << pos[iStep-1][1] << "   " << pos[iStep-1][2] << std::endl << std::endl;
        std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
        std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif
    }

    //Now the swimming is done, find the point with closest distance of approach, let iStep store the index of that step
    double dca_min = 1E9;
    double dca_xmin = 1E9;
    double dca_ymin = 1E9;

    iStep = NSTEPS_FMAG+NSTEPS_TARGET;   // set the default point to the most upstream
    int iStep_x = iStep;                  // the point when track cross beam line in X and in Y
    int iStep_y = iStep;                  // both intialized with the most upstream position
    for(int i = 0; i < NSTEPS_FMAG+NSTEPS_TARGET+1; ++i)
    {
        if(FMAGSTR*charge*mom[i].Px() < 0.) continue;    // this is the upstream accidental cross, ignore

        double dca = (pos[i] - TVector3(X_BEAM, Y_BEAM, pos[i].Z())).Perp();
        if(dca < dca_min)
        {
            dca_min = dca;
            iStep = i;
        }

        double dca_x = fabs(pos[i].X() - X_BEAM);
        if(dca_x < dca_xmin)
        {
            dca_xmin = dca_x;
            iStep_x = i;
        }

        double dca_y = fabs(pos[i].Y() - Y_BEAM);
        if(dca_y < dca_ymin)
        {
            dca_ymin = dca_y;
            iStep_y = i;
        }
    }

    setVertexFast(mom[iStep], pos[iStep]);
    setDumpFacePos(pos[NSTEPS_FMAG]);
    setDumpFaceMom(mom[NSTEPS_FMAG]);
    setTargetPos(fDumpFacePos + TVector3(fDumpFaceMom.Px()/fDumpFaceMom.Pz()*Z_TARGET, fDumpFaceMom.Py()/fDumpFaceMom.Pz()*Z_TARGET, Z_TARGET));
    setTargetMom(mom[NSTEPS_FMAG]);

    double dz_x = -pos[iStep_x].X()/mom[iStep_x].Px()*mom[iStep_x].Pz();
    setXVertexPos(pos[iStep_x] + TVector3(mom[iStep_x].Px()/mom[iStep_x].Pz()*dz_x, mom[iStep_x].Py()/mom[iStep_x].Pz()*dz_x, dz_x));
    setXVertexMom(mom[iStep_x]);

    double dz_y = -pos[iStep_y].Y()/mom[iStep_y].Py()*mom[iStep_y].Pz();
    setYVertexPos(pos[iStep_y] + TVector3(mom[iStep_y].Px()/mom[iStep_y].Pz()*dz_y, mom[iStep_y].Py()/mom[iStep_y].Pz()*dz_y, dz_y));
    setYVertexMom(mom[iStep_y]);

#ifdef _DEBUG_ON_LEVEL_2
    std::cout << "The one with minimum DCA is: " << iStep << ": " << std::endl;
    std::cout << mom[iStep][0]/mom[iStep][2] << "     " << mom[iStep][1]/mom[iStep][2] << "     " << mom[iStep][2] << "     ";
    std::cout << pos[iStep][0] << "  " << pos[iStep][1] << "   " << pos[iStep][2] << std::endl << std::endl;
#endif

#ifdef _ENABLE_KF
    if(hyptest) updateVtxHypothesis();
#endif

    if(cleanupPos) delete[] pos;
    if(cleanupMom) delete[] mom;
}

void SRecTrack::print(std::ostream& os) const
{
  os << "=============== Reconstructed track ==================" << std::endl;
  os << "This candidate has " << fHitIndex.size() << " hits!" << std::endl;
  os << "Most upstream momentum is: " << 1./fabs((fState[0])[0][0]) << std::endl;
  os << "Chi square of the track is: " << fChisq << std::endl;

  os << "Current vertex position: " << std::endl;
  for(Int_t i = 0; i < 3; i++) os << fVertexPos[i] << "  ";
  os << std::endl;

  os << "Momentum at vertex: " << 1./fabs(fStateVertex[0][0]) << std::endl;
  os << "Chi square at vertex: " << fChisqVertex << std::endl;
}

void SRecDimuon::calcVariables(int choice)
{
    TLorentzVector pos, neg;
    if(choice == 0)
    {
        pos = p_pos;
        neg = p_neg;
    }
    else if(choice == 1)
    {
        pos = p_pos_target;
        neg = p_neg_target;
    }
    else if(choice == 2)
    {
        pos = p_pos_dump;
        neg = p_neg_dump;
    }
    else
    {
        std::cout << "ERROR!!! In SRecDimuon::calcVariables(choice), only three dimuon vertex choice numbers are provided - ";
        std::cout << " 0 (default) = fitted vertex position, 1 = target position, 2 = dump position" << std::endl;
        
        //Unphysical error values
        mass = -1.;
        xF = -10.;
        x1 = -10.;
        x2 = -10.;
        pT = -1.;
        costh = -10.;
        phi = -10.;
        mass_single = -1.;

        return;
    }

    Double_t mp = 0.938;
    Double_t ebeam = 120.;

    TLorentzVector p_beam(0., 0., sqrt(ebeam*ebeam - mp*mp), ebeam);
    TLorentzVector p_target(0., 0., 0., mp);

    TLorentzVector p_cms = p_beam + p_target;
    TLorentzVector p_sum = pos + neg;

    mass = p_sum.M();
    pT = p_sum.Perp();

    x1 = (p_target*p_sum)/(p_target*p_cms);
    x2 = (p_beam*p_sum)/(p_beam*p_cms);

    Double_t s = p_cms.M2();
    TVector3 bv_cms = p_cms.BoostVector();
    p_sum.Boost(-bv_cms);
    xF = 2.*p_sum.Pz()/TMath::Sqrt(s)/(1. - mass*mass/s);

    costh = 2.*(p_neg.E()*pos.Pz() - pos.E()*neg.Pz())/mass/sqrt(mass*mass + pT*pT);
    phi = atan2(2.*sqrt(mass*mass + pT*pT)*(neg.X()*pos.Y() - pos.X()*neg.Y()), mass*(pos.X()*pos.X() - neg.X()*neg.X() + pos.Y()*pos.Y() - neg.Y()*neg.Y()));
    mass_single = (p_pos_single + p_neg_single).M();
}

int SRecDimuon::isValid() const
{
    //Chisq of vertex fit
    if(chisq_kf > 15. || chisq_kf < 0.) return false;

    //Kinematic cuts
    if(FMAGSTR*(p_pos.Px() - p_neg.Px()) < 0.) return false;
    if(fabs(xF) > 1.) return false;
    if(x1 < 0. || x1 > 1.) return false;
    if(x2 < 0. || x2 > 1.) return false;
    if(mass < 0. || mass > 10.) return false;
    if(fabs(vtx.X()) > 2.) return false;
    if(fabs(vtx.Y()) > 2.) return false;
    if(fabs(p_pos.Px() + p_neg.Px()) > 3.) return false;
    if(fabs(p_pos.Py() + p_neg.Py()) > 3.) return false;
    if(vtx.Z() > 200. || vtx.Z() < -300.) return false;
    if(p_pos.Pz() + p_neg.Pz() > 120. || p_pos.Pz() + p_neg.Pz() < 30.) return false;

    //Track separation cuts
    if(fabs(vtx_pos.Z() - vtx_neg.Z()) > 250.) return false;

    //Everything is fine
    return true;
}

bool SRecDimuon::isTarget()
{
    //single muon vertex
    if(vtx_pos.Z() > -100. || vtx_pos.Z() < -500.) return false;
    if(vtx_neg.Z() > -100. || vtx_neg.Z() < -500.) return false;

    //Track projection comparison
    double pzp = p_pos.Pz();
    if(proj_dump_pos.Perp() - proj_target_pos.Perp() < 9.44301-0.356141*pzp+0.00566071*pzp*pzp-3.05556e-05*pzp*pzp*pzp) return false;

    double pzm = p_neg.Pz();
    if(proj_dump_neg.Perp() - proj_target_neg.Perp() < 9.44301-0.356141*pzm+0.00566071*pzm*pzm-3.05556e-05*pzm*pzm*pzm) return false;

    return true;
}

bool SRecDimuon::isDump()
{
    //single muon vertex
    if(vtx_pos.Z() > 150.) return false;
    if(vtx_neg.Z() > 150.) return false;

    //Track projection comparison
    double pzp = p_pos.Pz();
    if(proj_target_pos.Perp() - proj_dump_pos.Perp() < 9.44301-0.356141*pzp+0.00566071*pzp*pzp-3.05556e-05*pzp*pzp*pzp) return false;

    double pzm = p_neg.Pz();
    if(proj_target_neg.Perp() - proj_dump_neg.Perp() < 9.44301-0.356141*pzm+0.00566071*pzm*pzm-3.05556e-05*pzm*pzm*pzm) return false;

    return true;
}

SRecEvent::SRecEvent()
{
    fRunID = -1;
    fSpillID = -1;
    fEventID = -1;
    fRecStatus = 0;

    fSource1 = -1;
    fSource2 = -1;

    clear();
}

void SRecEvent::setRawEvent(SRawEvent *rawEvent)
{
    clear();
    setEventInfo(rawEvent);

    for(Int_t i = 0; i < rawEvent->getNHitsAll(); i++)
    {
        fLocalID.insert(std::map<Int_t, Int_t>::value_type(rawEvent->getHit(i).index, i));
    }
}

void SRecEvent::setEventInfo(SRawEvent* rawEvent)
{
    fRunID = rawEvent->getRunID();
    fSpillID = rawEvent->getSpillID();
    fEventID = rawEvent->getEventID();

    fTriggerBits = rawEvent->getTriggerBits();
    fTargetPos = rawEvent->getTargetPos();
}

std::vector<Int_t> SRecEvent::getChargedTrackIDs(Int_t charge)
{
    std::vector<Int_t> trkIDs;
    trkIDs.clear();

    Int_t nTracks = getNTracks();
    for(Int_t i = 0; i < nTracks; i++)
    {
        if(fAllTracks[i].getCharge() == charge)
        {
            trkIDs.push_back(i);
        }
    }

    return trkIDs;
}

void SRecEvent::clear()
{
    fAllTracks.clear();
    fLocalID.clear();
    fDimuons.clear();

    fRecStatus = 0;
}

void SRecEvent::clearTracks()
{
    fAllTracks.clear();
    fLocalID.clear();
    fRecStatus = 0;
}

/// Clear the dimuon list.
/**
 * It should be called when only the vertexing is re-done.
 * But probably `fRecStatus` will be messed up.
 * We had better separate `fRecStatus` into `fRecStatusTracking` and `fRecStatusVertexing`.
 */
void SRecEvent::clearDimuons()
{
    fDimuons.clear();
}
