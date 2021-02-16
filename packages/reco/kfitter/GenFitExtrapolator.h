/*
GenFitExtrapolator.h

Class definition of GenFitExtrapolator, which is used to transport the track
through the detector and magnetic field and propagate the error matrix correctly.
The magnetic field integration and energy loss effect are included.

This class will be used in the prediction step of Kalman filter.

The whole algorithm is based on the Geant4e.
The detector construction is taken from GMC by Bryan Kerns.

Author: Kun Liu, liuk@fnal.gov
Created: 10-13-2011
*/

#ifndef _GENFITEXTRAPOLATOR_H
#define _GENFITEXTRAPOLATOR_H

#include <string>
#include <TMatrixD.h>
#include <TMatrixDSym.h>
#include <TVector3.h>

#include <GlobalConsts.h>

class TGeoManager;
class PHField;

class GenFitExtrapolator
{
public:
    GenFitExtrapolator();
    ~GenFitExtrapolator();

    ///Initialize geometry and physics
    //bool init(std::string geometrySchema, double fMagStr = FMAGSTR, double kMagStr = KMAGSTR);
    bool init(const PHField* field, const TGeoManager *geom);

    ///Set input initial state parameters
    void setInitialStateWithCov(double z_in, TMatrixD& state_in, TMatrixD& cov_in);

    ///Set particle type
    void setParticleType(int type);

    ///Get the final state parameters and covariance
    void getFinalStateWithCov(TMatrixD& state_out, TMatrixD& cov_out);
    double getTravelLength() { return travelLength; }

    ///Get the propagator
    //void buildNumericalPropagator();
    //void getNumericalPropagator(TMatrixD& prop);
    void getPropagator(TMatrixD& prop);

    /*
     * Extrapolate to a new surface z_out
     * \param z_out target position, in cm
     */
    bool extrapolateTo(double z_out);

    ///Extrapolate to the primary vertex
    double extrapolateToIP();

    ///Transformation between the state vector and the mom/pos
    void convertSVtoMP(double z, TMatrixD& state, TVector3& mom, TVector3& pos);
    void convertMPtoSV(TVector3& mom, TVector3& pos, TMatrixD& state);

    ///Transformation between the SC and SD parameters and error matrix
    ///Transplanted from GEANT3 fortran code
    void TRSDSC(int charge, TVector3 mom_input, TVector3 pos_input);
    void TRSCSD(int charge, TVector3 mom_input, TVector3 pos_input);

    TMatrixD& getJacSD2SC() { return jac_sd2sc; }
    TMatrixD& getJacSC2SD() { return jac_sc2sd; }

    ///External control of modes
    void setPropCalc(bool option) { calcProp = option; }
    void setLengthCalc(bool option) { calcLength = option; }


   ///Tranformation between GenFit and Legacy plane; Abi
    void TRGENFIT2LEGACY(int charge, TVector3 mom_input, TVector3 pos_input);
    void TRLEGACY2GENFIT(int charge, TVector3 mom_input, TVector3 pos_input);


    ///Debug print
    void print();

private:

    ///Internal static flag, check if the tracking manager has been inited of not
    static bool fullInit;

    ///Particle type, for now only mu+/- is implemented
    int iParType;
    enum PropDirection {Forward, Backward};
    PropDirection direction;

//    G4String parType;
//
//    ///Geant4 stuff
//    G4ErrorPropagatorManager *g4eMgr;
//    G4ErrorPropagatorData *g4eData;
//    G4ErrorPlaneSurfaceTarget *g4eTarget;
//    G4ErrorFreeTrajState *g4eState;
//    G4ErrorMatrix g4eProp;
//    G4ErrorMode g4eMode;
//
//    ///Initial state
//    G4ThreeVector pos_i;
//    G4ThreeVector mom_i;
//    G4ErrorTrajErr cov_i;
//
//    ///Final state
//    G4ThreeVector pos_f;
//    G4ThreeVector mom_f;
//    G4ErrorTrajErr cov_f;

    /// GenFit uses cm, GeV and kilogauss
    ///Initial state
    TVector3 pos_i;
    TVector3 mom_i;
    TMatrixDSym cov_i;

    ///Final state
    TVector3 pos_f;
    TVector3 mom_f;
    TMatrixDSym cov_f;

    ///Jacobians
    TMatrixD jac_sd2sc;
    TMatrixD jac_sc2sd;

    ///Control on calculation of propagation matrix
    bool calcProp;

    ///Control on calculation of travel length
    bool calcLength;
    double travelLength;


    ///Jacobians for genfit and lecay; Abi
    TMatrixD jac_genfit2legacy;
    TMatrixD jac_legacy2genfit;

    ///Propagator from genfit; Abi
    TMatrixD propM;    

    TGeoManager* _tgeo_manager;
};

#endif
