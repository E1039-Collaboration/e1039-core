/*==========================================================================
  Author: Abinash Pun, Kun Liu
  Sep, 2019
  Goal: Import the primary vertex generator of E906 experiment(DPVertexGenerator)
  from Kun to E1039 experiment in Fun4All framework
  ============================================================================*/

#ifndef __SQPRIMARYVERTEXGEN_H__
#define __SQPRIMARYVERTEXGEN_H__

#include <vector>
#include <memory>
#include <TString.h>
#include <TGeoManager.h>
#include <TF2.h>
#include <TVector3.h>
#include <TRandom1.h>
#include "SQBeamlineObject.h"

class PHCompositeNode;

/// Class to hold the profile of materials at a (x, y) beam position.
/**
 * Only used by `SQPrimaryVertexGen` internally.
 */
class MaterialProfile
{
public:
  MaterialProfile();
  int findInteractingPiece(double rndm);
  void calcProb();

public:
  unsigned int nPieces;
  double probSum;
  double accumulatedProbs[100];
  std::vector<SQBeamlineObject> interactables;
};

/// Class to generate the event vertex, based on the beam profile and the target+spectrometer materials given.
/**
 * This class is used inside the event generators like `SQPrimaryParticleGen`.
 * User need not instantiate (i.e. new) this class explicitly.
 *
 * The beam profile in (x, y) is Gaussian at r<3*sigma and 1/r at r>=3*sigma.
 *
 * User can control the beam profile and the materials by changing the following `recoConsts` flags.
 *  - `X0_TARGET`, `Y0_TARGET` ... The center location of the target material.
 *  - `RX_TARGET`, `RY_TARGET` ... The radius of the target material.
 *  - `X_BEAM`, `Y_BEAM`       ... The center location of the beam.
 *  - `SIGX_BEAM`, `SIGY_BEAM` ... The Gaussian width of the beam.
 *  - `VTX_GEN_MATERIAL_MODE`  ... Mode to enable only a part of materials.
 *    - `All`           ... Enable all materials (default).
 *    - `Target`        ... Enable only the target material.
 *    - `Dump`          ... Enable only the dump material.
 *    - `TargetDumpGap` ... Enable only the air gap between the target and the dump.
 *    - `Manual`        ... Enable only a z-range specified via `VTX_GEN_Z_START` and `VTX_GEN_Z_STOP`.
 */
class SQPrimaryVertexGen
{
  static constexpr double Z_MIN = -800.0;
  static constexpr double Z_MAX =  503.0;

public:
  SQPrimaryVertexGen();
  ~SQPrimaryVertexGen();

  //! Initialize at the begining of Run
  int InitRun(PHCompositeNode* node);
  int InitRun(TString filename);

  //! fill material profile using initial x/y position
  void fillMaterialProfile(MaterialProfile* prof, double xvtx, double yvtx);

  //! generate 3-D vertex position
  TVector3 generateVertex();

  //! use the beam profile to generate vertex in X-Y plane
  void generateVtxPerp(double& x, double& y);

  //! get the proton/neutron ratio of the piece, must be called after generateVertex
  double getPARatio() { return pARatio; }

  //! get the relative luminosity on this target
  double getLuminosity() { return probSum; }

  //! beam profile function
  static double funcBeamProfile(double* val, double* par);
  
private:
  //! Real initialization - should not be called directly
  void init();

  //! Find the z-boundaries of all materials at (xpos, ypos, z_min...z_max).
  void FindMaterialBoundaries(const double xpos, const double ypos, const double z_min, const double z_max, int& n_b, double* z_b);

  //! Find the z-range of the material given.
  void FindMaterialRange(double& z1, double& z2, const std::string name, const double xpos=0, const double ypos=0);

  //! Default material profile - pre-calculated and used for protons within the target acceptance
  MaterialProfile* defaultMatProf;
  
  //! cache of the properties of the material selected
  double pARatio;
  double probSum;

  //! Beam profile
  TF2* beamProfile;

  //! pointer to the geomManager
  TGeoManager* geoManager;

  //! Random number
  TRandom1 rndm;

  //! Pointer to the topNode to get geometry after PHG4Reco Init
  PHCompositeNode* topNode;

  //! flag signifying the vertex generator has been initialized
  bool inited;
  
  //! target profile parameters
  double targetX0;
  double targetY0;
  double targetSX;
  double targetSY;

  //! flag for using target and dump only
  bool targetOnlyMode;
  std::string material_mode;

  //! Start/Stop position for Z search
  double z_start;
  double z_stop;
};

#endif
