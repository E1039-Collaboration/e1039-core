/*
TrackExtrpolator.cxx

Implementation of class TrackExtrapolator

Author: Kun Liu, liuk@fnal.gov
Created: 10-13-2011
*/

#include <iostream>
#include <cmath>
#include <TMath.h>
#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include "Settings.hh"
#include "TrackExtrapolator.hh"
#include "TPhysicsList.hh"

bool TrackExtrapolator::fullInit = false;

TrackExtrapolator::TrackExtrapolator()
{
    cov_i = G4ErrorTrajErr(5, 0);
    cov_f = G4ErrorTrajErr(5, 0);
    g4eProp = G4ErrorMatrix(5, 5);

    jac_sd2sc.ResizeTo(5, 5);
    jac_sc2sd.ResizeTo(5, 5);
}

TrackExtrapolator::~TrackExtrapolator()
{
    //delete everything ...
    g4eMgr->CloseGeometry();
}

bool TrackExtrapolator::init(std::string geometrySchema, double fMagStr, double kMagStr)
{
    //Initialize propagate manager and related stuff
    //G4VSteppingVerbose::SetInstance(new G4SteppingVerbose);
    g4eMgr = G4ErrorPropagatorManager::GetErrorPropagatorManager();
    g4eData = G4ErrorPropagatorData::GetErrorPropagatorData();

    calcProp = true;
    calcLength = false;
    if(!fullInit)
    {
        //Specify the geometry schema for the MySQL
        Settings *mySettings = new Settings();
        mySettings->geometrySchema = geometrySchema.c_str();

        if(fabs(fMagStr) < 10.) mySettings->fMagMultiplier = fMagStr;
        if(fabs(kMagStr) < 10.) mySettings->kMagMultiplier = kMagStr;

        g4eMgr->SetUserInitialization(new DetectorConstruction(mySettings));
        g4eMgr->InitGeant4e();

        fullInit = true;
    }

    G4UImanager::GetUIpointer()->ApplyCommand("/control/verbose 0");
    G4UImanager::GetUIpointer()->ApplyCommand("/tracking/verbose 0");

    return true;
}

void TrackExtrapolator::setParticleType(int type)
{
    iParType = type;
    switch(type)
    {
    case 1:
        parType = "mu+";
        break;

    case -1:
        parType = "mu-";
        break;

    default:
        LogDebug("TrackExtrapolator: Particle type is wrong! ");
        break;
    }
}

void TrackExtrapolator::convertSVtoMP(double z, TMatrixD& state, G4ThreeVector& mom, G4ThreeVector& pos)
{
    double p = fabs(1./state[0][0]);
    double pz = p/sqrt(1. + state[1][0]*state[1][0] + state[2][0]*state[2][0]);
    double px = pz*state[1][0];
    double py = pz*state[2][0];

    double x = state[3][0];
    double y = state[4][0];

    pos.set(x*cm, y*cm, z*cm);
    mom.set(px*GeV, py*GeV, pz*GeV);
}

void TrackExtrapolator::convertMPtoSV(G4ThreeVector& mom, G4ThreeVector& pos, TMatrixD& state)
{
    G4ThreeVector mom_gev = mom*MeV/GeV;
    G4ThreeVector pos_cm = pos*mm/cm;

    state[0][0] = double(iParType)/mom_gev.mag();
    state[1][0] = mom_gev.x()/mom_gev.z();
    state[2][0] = mom_gev.y()/mom_gev.z();

    state[3][0] = pos_cm.x();
    state[4][0] = pos_cm.y();
}

void TrackExtrapolator::setInitialStateWithCov(double z_in, TMatrixD& state_in, TMatrixD& cov_in)
{
    //Convert (1/p, x', y', x, y) to 3-vectors of momentum and position
    convertSVtoMP(z_in, state_in, mom_i, pos_i);

    if(state_in[0][0] > 0)
    {
        setParticleType(1);
    }
    else
    {
        setParticleType(-1);
    }

    TMatrixD cov_sd(5, 5);
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            cov_sd[i][j] = cov_in[i][j];

            if(i == 0) cov_sd[i][j] = double(iParType)*cov_sd[i][j];
            if(j == 0) cov_sd[i][j] = double(iParType)*cov_sd[i][j];
        }
    }

    ///convert the error matrix from SD to SC
    TRSDSC(iParType, mom_i, pos_i);
    TMatrixD jac_sd2sc_T = jac_sd2sc;
    jac_sd2sc_T.T();

    TMatrixD cov_sc = jac_sd2sc*cov_sd*jac_sd2sc_T;
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            cov_i[i][j] = cov_sc[i][j];
        }
    }
}

bool TrackExtrapolator::extrapolateTo(double z_out)
{
    ///Convert the units to the internal unit system of Geant4
    z_out = z_out*cm;

    //LogDebug("Extrapolating from " << pos_i[2] << " to " << z_out);

    ///If the initial or final position is out of the reasonable world
    if(pos_i[2] > 24000 || pos_i[2] < -600*cm || z_out > 24000 || z_out < -600*cm)
    {
        return false;
    }

    ///if the initial and final z position is the same, don't make the transportation
    if(fabs(pos_i[2] - z_out) < 1E-3)
    {
        mom_f = mom_i;
        pos_f = pos_i;
        cov_f = cov_i;

        return true;
    }

    ///Set step size
    int step = 1;
    if(z_out < 5000.)
    {
        step = 50;
    }
    else if(fabs(z_out - pos_i[2]) > 1000.)
    {
        step = 50;
    }
    else
    {
        step = 4;
    }

    char buffer[100];
    sprintf(buffer, "/geant4e/limits/stepLength %d mm", step);
    G4UImanager::GetUIpointer()->ApplyCommand(buffer);

    ///Set direction of propagtion
    if(pos_i[2] < z_out)
    {
        g4eMode = G4ErrorMode_PropForwards;
    }
    else
    {
        g4eMode = G4ErrorMode_PropBackwards;
        for(int i = 0; i < 5; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                if(i == 1) cov_i[i][j] = -cov_i[i][j];
                if(j == 1) cov_i[i][j] = -cov_i[i][j];
                if(i == 3) cov_i[i][j] = -cov_i[i][j];
                if(j == 3) cov_i[i][j] = -cov_i[i][j];
            }
        }
    }

    ///Set the initial plane and final target
    g4eTarget = new G4ErrorPlaneSurfaceTarget(0., 0., 1., -z_out);
    g4eData->SetTarget(g4eTarget);

    ///Set initial trajctory state which is also used to store final state
    g4eState = new G4ErrorFreeTrajState(parType, pos_i, mom_i, cov_i);

    ///Make the propagation
    //LogDebug("============= Before propagation ==================");
    //print();

    int ierr = 0;
    if(g4eMode == G4ErrorMode_PropBackwards)
    {
        g4eState->SetMomentum(-g4eState->GetMomentum());
        ierr = propagate();
        g4eState->SetMomentum(-g4eState->GetMomentum());
    }
    else
    {
        ierr = propagate();
    }

    if(ierr != 0)
    {
        //LogDebug("Error code for this run from " << pos_i[2] << " to " << z_out << " is " << ierr);
        return false;
    }

    //LogDebug("============ After propagation ===================");

    //Extract final results
    pos_f = g4eState->GetPosition();
    mom_f = g4eState->GetMomentum();
    cov_f = g4eState->GetError();

    //print();

    ///Clean up the temporary objects
    delete g4eState;
    delete g4eTarget;

    return true;
}

int TrackExtrapolator::propagate()
{
    travelLength = 0.;

    ///If neither prop mtrix or travelLength is needed, then call the one step Propagate()
    if(!(calcProp || calcLength))
    {
        return g4eMgr->Propagate(g4eState, g4eTarget, g4eMode);
    }


    ///Initialize the prop. matrix if needed
    if(calcProp)
    {
        for(int i = 0; i < 5; i++)
        {
            for(int j = 0; j < 5; j++)
            {
                g4eProp[i][j] = 0.;
                if(i == j) g4eProp[i][j] = 1.;
            }
        }
    }

    ///Start the step-by-step propagation
    g4eMgr->InitTrackPropagation();
    G4ThreeVector pos_before, pos_after;

    bool isLastStep = false;
    while(!isLastStep)
    {
        if(calcLength) pos_before = g4eState->GetPosition();

        int ierr = g4eMgr->PropagateOneStep(g4eState, g4eMode);
        if(ierr != 0)
        {
            return ierr;
        }

        //calculate the propagation matrix
        if(calcProp)
        {
            G4ErrorMatrix g4eProp_oneStep = g4eState->GetTransfMat();
            G4ErrorMatrix g4eProp_temp = g4eProp_oneStep*g4eProp;
            g4eProp = g4eProp_temp;
        }

        //calculate the travel length
        if(calcLength)
        {
            pos_after = g4eState->GetPosition();
            if((pos_before[2] < FMAG_LENGTH*cm && pos_before[2] > 0) || (pos_after[2] < FMAG_LENGTH*cm && pos_after[2] > 0))
            {
                double length = (pos_before - pos_after).mag();
                if(pos_after[2] < 0)
                {
                    length *= fabs(pos_before[2]/(pos_before[2] - pos_after[2]));
                }
                else if(pos_before[2] > FMAG_LENGTH*cm)
                {
                    length *= fabs((pos_after[2] - FMAG_LENGTH*cm)/(pos_before[2] - pos_after[2]));
                }

                travelLength += length;
            }
        }

        //Check if the target plane is reached
        isLastStep = g4eMgr->GetPropagator()->CheckIfLastStep(g4eState->GetG4Track());
    }

    g4eMgr->GetPropagator()->InvokePostUserTrackingAction(g4eState->GetG4Track());
    return 0;
}

void TrackExtrapolator::getFinalStateWithCov(TMatrixD& state_out, TMatrixD& cov_out)
{
    convertMPtoSV(mom_f, pos_f, state_out);

    TMatrixD cov_sc(5, 5);
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            cov_sc[i][j] = cov_f[i][j];

            if(g4eMode == G4ErrorMode_PropBackwards)
            {
                if(i == 1) cov_sc[i][j] = -cov_sc[i][j];
                if(j == 1) cov_sc[i][j] = -cov_sc[i][j];
                if(i == 3) cov_sc[i][j] = -cov_sc[i][j];
                if(j == 3) cov_sc[i][j] = -cov_sc[i][j];
            }
        }
    }

    //convert from SC to SD error matrix
    TRSCSD(iParType, mom_f, pos_f);
    TMatrixD jac_sc2sd_T = jac_sc2sd;
    jac_sc2sd_T.T();

    cov_out = jac_sc2sd*cov_sc*jac_sc2sd_T;

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            if(i == 0) cov_out[i][j] = double(iParType)*cov_out[i][j];
            if(j == 0) cov_out[i][j] = double(iParType)*cov_out[i][j];
        }
    }
}

void TrackExtrapolator::getPropagator(TMatrixD& prop)
{
    if(fabs(pos_i[2] - pos_f[2]) < 1E-3)
    {
        prop.UnitMatrix();
        return;
    }

    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            prop[i][j] = g4eProp[i][j];

            if(i == 0) prop[i][j] = double(iParType)*prop[i][j];
            if(j == 0) prop[i][j] = double(iParType)*prop[i][j];
            if(g4eMode == G4ErrorMode_PropBackwards)
            {
                if(i == 1) prop[i][j] = -prop[i][j];
                if(j == 1) prop[i][j] = -prop[i][j];
                if(i == 3) prop[i][j] = -prop[i][j];
                if(j == 3) prop[i][j] = -prop[i][j];
            }
        }
    }

    prop = jac_sc2sd*prop*jac_sd2sc;
}

void TrackExtrapolator::TRSDSC(int charge, G4ThreeVector mom_input, G4ThreeVector pos_input)
{
    ///convert the internal momentum and positon to ROOT vectors and use GeV and cm
    TVector3 mom(mom_input[0], mom_input[1], mom_input[2]);
    TVector3 pos(pos_input[0], pos_input[1], pos_input[2]);

    mom *= 0.001;
    pos *= 0.1;

    ///Define the V and W direction of SD coordinate
    TVector3 DJ(1., 0., 0.);
    TVector3 DK(0., 1., 0.);
    TVector3 DI = DJ.Cross(DK);

    ///Calculate the representation of momentum
    double p_inv = 1./mom.Mag();
    double vp = mom.Dot(DJ)/mom.Dot(DI);
    double wp = mom.Dot(DK)/mom.Dot(DI);
    double lambda = TMath::Pi()/2. - mom.Theta();
    //double phi = mom.Phi();

    TVector3 TVW;
    TVW.SetX(1./sqrt(1. + vp*vp + wp*wp));
    TVW.SetY(vp*TVW.X());
    TVW.SetZ(wp*TVW.X());

    TVector3 TN;
    for(int i = 0; i < 3; i++)
    {
        TN[i] = TVW[0]*DI[i] + TVW[1]*DJ[i] + TVW[2]*DK[i];
    }

    double cosl = cos(lambda);
    double cosl1 = 1./cosl;

    TVector3 UN(-TN.Y()*cosl1, TN.X()*cosl1, 0.);
    TVector3 VN(-TN.Z()*UN.Y(), TN.Z()*UN.X(), cosl);

    double UJ = UN.Dot(DJ);
    double UK = UN.Dot(DK);
    double VJ = VN.Dot(DJ);
    double VK = VN.Dot(DK);

    jac_sd2sc.Zero();
    jac_sd2sc[0][0] = 1.;
    jac_sd2sc[1][1] = TVW[0]*VJ;
    jac_sd2sc[1][2] = TVW[0]*VK;
    jac_sd2sc[2][1] = TVW[0]*UJ*cosl1;
    jac_sd2sc[2][2] = TVW[0]*UK*cosl1;
    jac_sd2sc[3][3] = UJ;
    jac_sd2sc[3][4] = UK;
    jac_sd2sc[4][3] = VJ;
    jac_sd2sc[4][4] = VK;

    const G4Field* field = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();
    if(charge != 0 && field)
    {
        double pos_cm[3];
        pos_cm[0] = pos.X()*cm;
        pos_cm[1] = pos.Y()*cm;
        pos_cm[2] = pos.Z()*cm;

        double H_temp[3];
        field->GetFieldValue(pos_cm, H_temp);
        TVector3 H(H_temp[0], H_temp[1], H_temp[2]);
        H = H*(10./tesla);

        double HA = H.Mag();
        double HAM = HA*p_inv;
        double HM;
        if(HA < 1E-6)
        {
            HM = 0.;
        }
        else
        {
            HM = charge/HA;
        }

        double Q = -HAM*c_light/(km/ns);
        double sinz = -H.Dot(UN)*HM;
        double cosz = H.Dot(VN)*HM;

        jac_sd2sc[1][3] = -Q*TVW[1]*sinz;
        jac_sd2sc[1][4] = -Q*TVW[2]*sinz;
        jac_sd2sc[2][3] = -Q*TVW[1]*cosz*cosl1;
        jac_sd2sc[2][4] = -Q*TVW[2]*cosz*cosl1;
    }
}

void TrackExtrapolator::TRSCSD(int charge, G4ThreeVector mom_input, G4ThreeVector pos_input)
{
    ///convert the internal momentum and positon to ROOT vectors and use GeV and cm
    TVector3 mom(mom_input[0], mom_input[1], mom_input[2]);
    TVector3 pos(pos_input[0], pos_input[1], pos_input[2]);

    mom *= 0.001;
    pos *= 0.1;

    ///Define the V and W direction of SD coordinate
    TVector3 DJ(1., 0., 0.);
    TVector3 DK(0., 1., 0.);
    TVector3 DI = DJ.Cross(DK);

    ///Calculate the representation of momentum
    double p_inv = 1./mom.Mag();
    //double vp = mom.Dot(DJ)/mom.Dot(DI);
    //double wp = mom.Dot(DK)/mom.Dot(DI);
    double lambda = TMath::Pi()/2. - mom.Theta();
    double phi = mom.Phi();

    double cosl = cos(lambda);
    double sinp = sin(phi);
    double cosp = cos(phi);

    TVector3 TN(cosl*cosp, cosl*sinp, sin(lambda));
    TVector3 TVW;
    TVW.SetX(TN.Dot(DI));
    TVW.SetY(TN.Dot(DJ));
    TVW.SetZ(TN.Dot(DK));

    double T1R = 1./TVW[0];
    double T2R = T1R*T1R;
    TVector3 UN(-sinp, cosp, 0.);
    TVector3 VN(-TN.Z()*UN.Y(), TN.Z()*UN.X(), cosl);

    double UJ = UN.Dot(DJ);
    double UK = UN.Dot(DK);
    double VJ = VN.Dot(DJ);
    double VK = VN.Dot(DK);
    double UI = UN.Dot(DI);
    double VI = VN.Dot(DI);

    jac_sc2sd.Zero();
    jac_sc2sd[0][0] = 1.;
    jac_sc2sd[1][1] = -UK*T2R;
    jac_sc2sd[1][2] = VK*cosl*T2R;
    jac_sc2sd[2][1] = UJ*T2R;
    jac_sc2sd[2][2] = -VJ*cosl*T2R;
    jac_sc2sd[3][3] = VK*T1R;
    jac_sc2sd[3][4] = -UK*T1R;
    jac_sc2sd[4][3] = -VJ*T1R;
    jac_sc2sd[4][4] = UJ*T1R;

    const G4Field* field = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();
    if(charge != 0 && field)
    {
        double pos_cm[3];
        pos_cm[0] = pos.X()*cm;
        pos_cm[1] = pos.Y()*cm;
        pos_cm[2] = pos.Z()*cm;

        double H_temp[3];
        field->GetFieldValue(pos_cm, H_temp);
        TVector3 H(H_temp[0], H_temp[1], H_temp[2]);
        H = H*(10./tesla);

        double HA = H.Mag();
        double HAM = HA*p_inv;
        double HM;
        if(HA < 1E-6)
        {
            HM = 0.;
        }
        else
        {
            HM = charge/HA;
        }

        double Q = -HAM*c_light/(km/ns);
        double sinz = -H.Dot(UN)*HM;
        double cosz = H.Dot(VN)*HM;
        double T3R = Q*T1R*T1R*T1R;

        jac_sc2sd[1][3] = -UI*(VK*cosz - UK*sinz)*T3R;
        jac_sc2sd[1][4] = -VI*(VK*cosz - UK*sinz)*T3R;
        jac_sc2sd[2][3] = UI*(VJ*cosz - UJ*sinz)*T3R;
        jac_sc2sd[2][4] = VI*(VJ*cosz - UJ*sinz)*T3R;
    }
}

double TrackExtrapolator::extrapolateToIP()
{
    //Store the steps on each point
    G4ThreeVector mom[NSLICES_FMAG + NSTEPS_TARGET + 1];
    G4ThreeVector pos[NSLICES_FMAG + NSTEPS_TARGET + 1];

    //Step size in FMAG/target area, unit is cm.
    double step_fmag = FMAG_LENGTH/NSLICES_FMAG*cm;
    double step_target = fabs(Z_UPSTREAM)/NSTEPS_TARGET*cm;

    //Start from FMAG face downstream
    extrapolateTo(FMAG_LENGTH);
    pos[0] = pos_f;
    mom[0] = mom_f;

    //Now make the real swimming
    int iStep = 1;
    for(; iStep <= NSLICES_FMAG; ++iStep)
    {
        pos_i = pos[iStep-1];
        mom_i = mom[iStep-1];

        extrapolateTo((pos_i[2] - step_fmag)*mm/cm);

        pos[iStep] = pos_f;
        mom[iStep] = mom_f;
    }

    for(; iStep < NSLICES_FMAG+NSTEPS_TARGET+1; ++iStep)
    {
        pos_i = pos[iStep-1];
        mom_i = mom[iStep-1];

        extrapolateTo((pos_i[2] - step_target)*mm/cm);

        pos[iStep] = pos_f;
        mom[iStep] = mom_f;
    }

    //Find the one step with minimum DCA
    double dca_min = 1E6;
    for(int i = 0; i < NSLICES_FMAG+NSTEPS_TARGET+1; ++i)
    {
        double dca = sqrt(pos[i][0]*pos[i][0] + pos[i][1]*pos[i][1]);
        if(dca < dca_min)
        {
            dca_min = dca;
            iStep = i;
        }
    }

    return pos[iStep][2]*mm/cm;
}

void TrackExtrapolator::print()
{
    cout << "Propagating " << parType << ":" << endl;
    cout << "From " << pos_i << " to " << pos_f << endl;
    cout << "Momentum change: " << mom_i << " to " << mom_f << endl;

    cout << "Initial error matrix: " << endl;
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            cout << cov_i[i][j] << "  ";
        }
        cout << endl;
    }

    cout << "Final error matrix: " << endl;
    for(int i = 0; i < 5; i++)
    {
        for(int j = 0; j < 5; j++)
        {
            cout << cov_f[i][j] << "  ";
        }
        cout << endl;
    }
}
