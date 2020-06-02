#ifndef _GLOBALCONSTS_H
#define _GLOBALCONSTS_H

//--------------- Geometry setup -----------------
#define nStations 7
#define nChamberPlanes 30
#define nHodoPlanes 16
#define nPropPlanes 8
#define nDarkPhotonPlanes 8

#define Z_KMAG_BEND 1064.26
#define Z_FMAG_BEND 251.4
#define Z_KFMAG_BEND 375.
#define ELOSS_KFMAG 8.12
#define ELOSS_ABSORBER 1.81
#define Z_ST2 1347.36
#define Z_ABSORBER 2028.19
#define Z_REF 0.
#define Z_TARGET -300.00
#define Z_DUMP 42.
#define Z_ST1 600.
#define Z_ST3 1910.

#define FMAG_HOLE_LENGTH 27.94
#define FMAG_HOLE_RADIUS 1.27
#define FMAG_LENGTH 502.92
#define Z_UPSTREAM -500.
#define Z_DOWNSTREAM 500.

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
