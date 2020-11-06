/*======================================================================================================
Author: Abinash Pun, Kun Liu
Sep, 2019
Goal: Beamline volumes work to import the primary vertex generator of E906 experiment(DPVertexGenerator)
from Kun to E1039 experiment in Fun4All framework
=======================================================================================================*/

#include "SQBeamlineObject.h"
#include "TGeoPhysicalConstants.h"
#include <TMath.h>
#include <TRandom3.h>
#include <phool/PHRandomSeed.h>

SQBeamlineObject::SQBeamlineObject() {}

SQBeamlineObject::SQBeamlineObject(const TGeoMaterial* pMaterial)
{
  //std::cout << " beamline material: "<< pMaterial->GetName() << std::endl;
  nucIntLen = pMaterial->GetIntLen();
  density = pMaterial->GetDensity();


  TGeoMixture* pMaterialmix;
  pMaterialmix = (TGeoMixture*)pMaterial;
  //pMaterial->Print("");
  //pMaterialmix->Print(" ");  
 
  Z=pMaterial->GetZ();
  A=pMaterial->GetA();
  N=A-Z;
}

bool SQBeamlineObject::operator < (const SQBeamlineObject& obj) const
{
    return z0 < obj.z0;
}

std::ostream& operator << (std::ostream& os, const SQBeamlineObject& obj)
{
  
 os << "Beamline object name " << obj.name << " at " << obj.z_up << " <-- " << obj.z0 << " --> " << obj.z_down << "\n"
       << "   Z = " << obj.Z << ", A = " << obj.A << ", N = " << obj.N << "\n"
       << "   Nuclear inc. len. = " << obj.nucIntLen << ", density = " << obj.density << "\n"
       << "   " << obj.length/obj.nucIntLen*100. << "% interaction length, " << " upstream attenuation = " << 1. - obj.attenuation/obj.attenuationSelf << "\n"
       << "   Attenuation by itself = " << obj.attenuationSelf << ", " << " real attenuation = " << obj.attenuation << "\n"
       << "   Collision prob = " << obj.prob << ", accumulated prob = " << obj.accumulatedProb;
 return os;

}

double SQBeamlineObject::getZ()
{

  double returnZ = 0.;
  unsigned int iseed = PHRandomSeed();
  gRandom->SetSeed(iseed);
  returnZ =  z_up - nucIntLen*TMath::Log(1. - attenuationSelf*gRandom->Uniform(0,1)); //Todo Fix this place holders
   
  return returnZ;
}

bool SQBeamlineObject::inAcceptance(double x, double y)
{
  return true;
  //return x*x/1.9/1.9 + y*y/2.1/2.1 < 1.;
}
