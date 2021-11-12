/*====================================================================
  Author: Abinash Pun, Kun Liu
  Sep, 2019
  Goal: Import the primary vertex generator of E906 experiment(DPVertexGenerator)
  from Kun to E1039 experiment in Fun4All framework
  =========================================================================*/

#include <iostream>

#include <TGeoManager.h>
#include <TMath.h>

#include <phgeom/PHGeomUtility.h>
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include "SQPrimaryVertexGen.h"


MaterialProfile::MaterialProfile():
  nPieces(0),
  probSum(0.)
{
}

int MaterialProfile::findInteractingPiece(double rndm)
{
  return TMath::BinarySearch(nPieces+1, accumulatedProbs, rndm);
}

void MaterialProfile::calcProb()
{
  //sort the interactable pieces first
  sort(interactables.begin(), interactables.end());

  //set the quanties that rely on its neighbours
  double attenuationSum = 0.;
  for(unsigned int i = 0; i < interactables.size(); ++i)
  {
    interactables[i].attenuationSelf = 1. - TMath::Exp(-interactables[i].length/interactables[i].nucIntLen);
    interactables[i].attenuation = (1. - attenuationSum)*interactables[i].attenuationSelf;
    interactables[i].prob = interactables[i].attenuation*interactables[i].density*interactables[i].nucIntLen;

    attenuationSum += interactables[i].attenuation;
  }

  //set the accumulatedProbs
  nPieces = interactables.size();
 
  interactables[0].accumulatedProb = 0.;
  accumulatedProbs[0] = 0.;
  for(unsigned int i = 1; i < nPieces; ++i)
  {
    interactables[i].accumulatedProb = interactables[i-1].accumulatedProb + interactables[i-1].prob;      
    accumulatedProbs[i] = interactables[i].accumulatedProb;   
  }
  accumulatedProbs[nPieces] = accumulatedProbs[nPieces-1] + interactables.back().prob;
  probSum = accumulatedProbs[nPieces];
    
  //Normalize the accumulated probability
  for(unsigned int i = 0; i < nPieces+1; ++i)
  {
    accumulatedProbs[i] = accumulatedProbs[i]/accumulatedProbs[nPieces];
  }
}

SQPrimaryVertexGen::SQPrimaryVertexGen():
  beamProfile(nullptr),
  geoManager(nullptr),
  defaultMatProf(nullptr),
  pARatio(0.),
  probSum(0.),
  inited(false),
  topNode(nullptr),
  targetOnlyMode(false),
  dumpOnlyMode(false),
  z_start(-800.),
  z_stop(503.)
{
}

SQPrimaryVertexGen::~SQPrimaryVertexGen()
{
  if(beamProfile    != nullptr) delete beamProfile;
  if(defaultMatProf != nullptr) delete defaultMatProf;
}

int SQPrimaryVertexGen::InitRun(PHCompositeNode* node)
{
  topNode = node;
  return 0;
}

int SQPrimaryVertexGen::InitRun(TString filename)
{
  TGeoManager::Import(filename);
  geoManager = gGeoManager;
  return 0;
}

void SQPrimaryVertexGen::init()
{
  recoConsts* rc = recoConsts::instance();

  // initialize target location/shape parameters
  targetX0 = rc->get_DoubleFlag("X0_TARGET");
  targetY0 = rc->get_DoubleFlag("Y0_TARGET");
  targetSX = rc->get_DoubleFlag("RX_TARGET");
  targetSY = rc->get_DoubleFlag("RY_TARGET");

  // initialize beam profile
  beamProfile = new TF2("beamProfile", &SQPrimaryVertexGen::funcBeamProfile, -6., 6., -6., 6., 4, 2);
  beamProfile->SetNpx(200);//SetNpx and Npy for smooth distribution (default is 30)
  beamProfile->SetNpy(200); 
 
  beamProfile->SetParameter(0, rc->get_DoubleFlag("X_BEAM"));
  beamProfile->SetParameter(1, rc->get_DoubleFlag("SIGX_BEAM"));
  beamProfile->SetParameter(2, rc->get_DoubleFlag("Y_BEAM"));
  beamProfile->SetParameter(3, rc->get_DoubleFlag("SIGY_BEAM"));

  rndm.SetSeed(PHRandomSeed());

  //If inited from a root file, then skip this step, so that it can be used independent of the Fun4All
  if(geoManager == nullptr) geoManager = PHGeomUtility::GetTGeoManager(topNode);

  // initialize target/dump only mode
  if(rc->get_BoolFlag("TARGETONLY"))
  {
    set_targetOnlyMode();
  }
  else if(rc->get_BoolFlag("DUMPONLY"))
  {
    set_dumpOnlyMode();
  }

  //Read the default 
  defaultMatProf = new MaterialProfile;
  fillMaterialProfile(defaultMatProf, targetX0, targetY0);

  inited = true;
}

void SQPrimaryVertexGen::set_targetOnlyMode()
{
  targetOnlyMode = true;
  z_start = -304;
  z_stop  = -296;
}

void SQPrimaryVertexGen::set_dumpOnlyMode() 
{
  dumpOnlyMode = true;
  z_start = -100.;
  z_stop = 503.;
}

void SQPrimaryVertexGen::fillMaterialProfile(MaterialProfile* prof, double xvtx, double yvtx)
{
  //std::cout << xvtx << "  " << yvtx << std::endl;
  //retrieve the z position of all geometry boundaries
  recoConsts* rc = recoConsts::instance();

  double z[100];
  int nBoundaries = 0;

  double eps = 1.E-6;
  double z_curr = z_start;  //not beyond collimator(z= -602.36 cm, lenth=121.92 cm) @Abi //-500.;
  z[0] = z_curr;
  while(z_curr < z_stop)
  {
    geoManager->IsSameLocation(xvtx, yvtx, z_curr, true);
    geoManager->SetCurrentDirection(0., 0., 1.);

    TGeoNode* node = geoManager->FindNextBoundary(600.);
    double z_step  = geoManager->GetStep();

    if(z_step > eps)
    {
      z_curr += z_step;
      z[++nBoundaries] = z_curr; 
    }
    else
    {
      z_curr += eps;
    }
  }

  //use the z boundaries to create interactibles
  for(int i = 0; i < nBoundaries; ++i)
  {
    double z_center = 0.5*(z[i] + z[i+1]);
    if(z_center > z_stop) continue;

    SQBeamlineObject obj(geoManager->FindNode(xvtx, yvtx, z_center)->GetVolume()->GetMaterial());
    TString name = geoManager->FindNode(xvtx, yvtx, z_center)->GetVolume()->GetName();
    obj.z_up = z[i];
    obj.z_down = z[i+1];
    obj.z0 = z_center;
    obj.length = z[i+1] - z[i];

    prof->interactables.push_back(obj);
  }

  //calculate all related properties
  prof->calcProb();

  // for(int i = 0; i < prof->nPieces; ++i)
  // {
  //   std::cout << prof->interactables[i] << std::endl;
  // }
  // return 0;
}


TVector3 SQPrimaryVertexGen::generateVertex()
{
  if(!inited) init();

  double xvtx, yvtx, zvtx;
  generateVtxPerp(xvtx, yvtx);
  bool inTarget = ((xvtx-targetX0)*(xvtx-targetX0)/targetSX/targetSX + (yvtx-targetY0)*(yvtx-targetX0)/targetSY/targetSY) < 1.;

  //keep generating x/y until it falls in target acceptance in targetOnlyMode
  while(targetOnlyMode && (!inTarget))
  {
    generateVtxPerp(xvtx, yvtx);
    inTarget = ((xvtx-targetX0)*(xvtx-targetX0)/targetSX/targetSX + (yvtx-targetY0)*(yvtx-targetX0)/targetSY/targetSY) < 1.;
  }

  // if the x/y is within target acceptance (should happen majority of the time), use the pre-cached 
  // material profile to generate z vtx; otherwise use geometry navigator to find the material profile /
  // on the path of this proton
  MaterialProfile* activeProf;
  if(!inTarget)
  {
    activeProf = new MaterialProfile;
    fillMaterialProfile(activeProf, xvtx, yvtx);
  }
  else
  {
    activeProf = defaultMatProf;
  }

  int index = activeProf->findInteractingPiece(rndm.Rndm());
  zvtx = activeProf->interactables[index].getZ(rndm.Rndm());
  pARatio = activeProf->interactables[index].protonPerc();
  probSum = activeProf->probSum;

  if(!inTarget) delete activeProf;
  return TVector3(xvtx, yvtx, zvtx);
}

void SQPrimaryVertexGen::generateVtxPerp(double& x, double& y)
{
  beamProfile->GetRandom2(x, y);
}

//!Function for beam profile (double gaussian at core and 1/r at tail)
double SQPrimaryVertexGen::funcBeamProfile(double* val, double* par)
{
  double x   = val[0];
  double y   = val[1];
  double x_c = par[0];
  double x_s = par[1];
  double y_c = par[2];
  double y_s = par[3];
  
  const double r_pipe = 5.0; // in cm
  const double r_boun = 3.0; // normalized boundary, where gaus changes to 1/r
  if(x*x + y*y > r_pipe*r_pipe) { // not "x-x_c" nor "y-y_c" here
    return 0.0;
  }

  double x_nr = (x - x_c)/x_s; // normalized relative x
  double y_nr = (y - y_c)/y_s; // normalized relative y
  double r2_nr = x_nr*x_nr + y_nr*y_nr;
  double r_nr  = sqrt(r2_nr);
  if(r_nr < r_boun*r_boun) {
    return exp(-0.5*r2_nr);
  } else { // >= r_boun
    return exp(-0.5*r_boun*r_boun)*r_boun/r_nr;
  }
}
