/*
TrackExtrapolator.h

Class definition of TrackExtrapolator, which is used to transport the track
through the detector and magnetic field and propagate the error matrix correctly.
The magnetic field integration and energy loss effect are included.

This class will be used in the prediction step of Kalman filter.

The whole algorithm is based on the Geant4e.
The detector construction is taken from GMC by Bryan Kerns.

Author: Kun Liu, liuk@fnal.gov
Created: 10-13-2011
*/

#ifndef _TRACKEXTRAPOLATOR_H
#define _TRACKEXTRAPOLATOR_H

#include "G4ErrorPropagator.hh"
#include "G4ErrorPropagatorData.hh"
#include "G4ErrorPropagatorManager.hh"
#include "G4ErrorPlaneSurfaceTarget.hh"
#include "G4ErrorSurfaceTrajState.hh"
#include "G4ErrorTrajErr.hh"

#include "G4SteppingVerbose.hh"
#include "G4UImanager.hh"
#include "G4VSteppingVerbose.hh"

#include <string>
#include <TMatrixD.h>
#include <TVector3.h>

#include "DetectorConstruction.hh"
#include "GlobalConsts.h"

#define LogDebug(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl

class TrackExtrapolator
{
public:
    TrackExtrapolator();
    ~TrackExtrapolator();

    ///Initialize geometry and physics
    bool init(std::string geometrySchema, double fMagStr = FMAGSTR, double kMagStr = KMAGSTR);

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

    ///Extrapolate to a new surface z_out
    bool extrapolateTo(double z_out);
    int propagate();

    ///Extrapolate to the primary vertex
    double extrapolateToIP();

    ///Transformation between the state vector and the mom/pos
    void convertSVtoMP(double z, TMatrixD& state, G4ThreeVector& mom, G4ThreeVector& pos);
    void convertMPtoSV(G4ThreeVector& mom, G4ThreeVector& pos, TMatrixD& state);

    ///Transformation between the SC and SD parameters and error matrix
    ///Transplanted from GEANT3 fortran code
    void TRSDSC(int charge, G4ThreeVector mom_input, G4ThreeVector pos_input);
    void TRSCSD(int charge, G4ThreeVector mom_input, G4ThreeVector pos_input);
    TMatrixD& getJacSD2SC() { return jac_sd2sc; }
    TMatrixD& getJacSC2SD() { return jac_sc2sd; }

    ///External control of modes
    void setPropCalc(bool option) { calcProp = option; }
    void setLengthCalc(bool option) { calcLength = option; }

    ///Debug print
    void print();

private:

    ///Internal static flag, check if the tracking manager has been inited of not
    static bool fullInit;

    ///Particle type, for now only mu+/- is implemented
    int iParType;
    G4String parType;

    ///Geant4 stuff
    G4ErrorPropagatorManager *g4eMgr;
    G4ErrorPropagatorData *g4eData;
    G4ErrorPlaneSurfaceTarget *g4eTarget;
    G4ErrorFreeTrajState *g4eState;
    G4ErrorMatrix g4eProp;
    G4ErrorMode g4eMode;

    ///Initial state
    G4ThreeVector pos_i;
    G4ThreeVector mom_i;
    G4ErrorTrajErr cov_i;

    ///Final state
    G4ThreeVector pos_f;
    G4ThreeVector mom_f;
    G4ErrorTrajErr cov_f;

    ///Jacobians
    TMatrixD jac_sd2sc;
    TMatrixD jac_sc2sd;

    ///Control on calculation of propagation matrix
    bool calcProp;

    ///Control on calculation of travel length
    bool calcLength;
    double travelLength;
};

#endif
