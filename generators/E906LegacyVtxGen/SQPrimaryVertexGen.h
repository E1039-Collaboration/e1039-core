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

class SQPrimaryVertexGen
{
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
  
  //! setting target/fmag only function
  void set_targetOnlyMode();
  void set_dumpOnlyMode();

private:
  //! Real initialization - should not be called directly
  void init();

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
  bool dumpOnlyMode;

  //! Start/Stop position for Z search
  double z_start;
  double z_stop;
};

#endif
