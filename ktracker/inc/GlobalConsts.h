#ifndef _GLOBALCONSTS_H
#define _GLOBALCONSTS_H

//--------------- Mode controls, experts only  ------------------
//Will be removed eventually
//=== Enable multiple minimizer feature, disabled by default
//#define _ENABLE_MULTI_MINI

//=== Coarse mode, no driftTime info is used, disabled by default
//#define COARSE_MODE

//---------------------FOLLOWING PART SHOULD NOT BE CHANGED FOR NO GOOD REASON -----------------------
//--------------- Fast tracking configuration ----
#define TX_MAX 0.15
#define TY_MAX 0.1
#define X0_MAX 150.
#define Y0_MAX 50.
#define INVP_MIN 0.01
#define INVP_MAX 0.2
#define PROB_LOOSE 0.0
#define PROB_TIGHT 1E-12
#define HIT_REJECT 3.

#define SAGITTA_TARGET_CENTER 1.85
#define SAGITTA_TARGET_WIN 0.25
#define SAGITTA_DUMP_CENTER 1.5
#define SAGITTA_DUMP_WIN 0.3

//--------------- Muon identification -----------
#define MUID_REJECT 4.
#define MUID_THE_P0 0.11825
#define MUID_EMP_P0 0.00643
#define MUID_EMP_P1 -0.00009
#define MUID_EMP_P2 0.00000046
#define MUID_Z_REF 2028.19
#define MUID_R_CUT 3.0

//--------------- Geometry setup -----------------
#define nStations 7
#define nChamberPlanes 30
#define nHodoPlanes 16
#define nPropPlanes 8

#define FMAGSTR -1.054
#define KMAGSTR -0.951

#define Z_KMAG_BEND 1064.26
#define Z_FMAG_BEND 251.4
#define Z_KFMAG_BEND 375.
#define PT_KICK_FMAG 2.909*FMAGSTR
#define PT_KICK_KMAG 0.4016*KMAGSTR
#define ELOSS_KFMAG 8.12
#define ELOSS_ABSORBER 1.81
#define Z_ST2 1347.36
#define Z_ABSORBER 2028.19
#define Z_REF 0.
#define Z_TARGET -129.54
#define Z_DUMP 42.
#define Z_ST1 600.
#define Z_ST3 1910.
#define RESOLUTION_DC 1.6

#define X_VTX       0.
#define Y_VTX       1.6
#define BEAM_SPOT_X 0.5
#define BEAM_SPOT_Y 0.5

#define MERGE_THRES 0.015

//-------------- Coarse swim setup --------------
#define FMAG_HOLE_LENGTH 27.94
#define FMAG_HOLE_RADIUS 1.27
#define FMAG_LENGTH 502.92
#define NSLICES_FMAG 100
#define NSTEPS_TARGET 200
#define ELOSS_FMAG_P0 7.18274
#define ELOSS_FMAG_P1 0.0361447
#define ELOSS_FMAG_P2 -0.000718127
#define ELOSS_FMAG_P3 7.97312e-06
#define ELOSS_FMAG_P4 -3.05481e-08
#define Z_UPSTREAM -500.
#define Z_DOWNSTREAM 500.

//-------------- Trigger analyzer modes ---------
#define USE_TRIGGER_HIT 1
#define USE_HIT 2

//-------------- Track finding exit code ---------------
#define TFEXIT_SUCCESS 0;
#define VFEXIT_SUCCESS 0;
#define TFEXIT_FAIL_MULTIPLICITY -1;
#define TFEXIT_FAIL_ROUGH_MUONID -2;
#define TFEXIT_FAIL_ST2_TRACKLET -3;
#define TFEXIT_FAIL_ST3_TRACKLET -4;
#define TFEXIT_FAIL_BACKPARTIAL -5;
#define TFEXIT_FAIL_GLOABL -6;
#define TFEXIT_FAIL_NO_DIMUON -7;
#define VFEXIT_FAIL_DIMUONPAIR -10;
#define VFEXIT_FAIL_ITERATION -20;

//-------------- Useful marcros -----------------
#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl
#define varName(x) #x

#endif
