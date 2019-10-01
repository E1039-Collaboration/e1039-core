#include "BeamlineObject.h"
#include "TGeoPhysicalConstants.h"
#include <TMath.h>
#include <TRandom3.h>

BeamlineObject::BeamlineObject() {}

BeamlineObject::BeamlineObject(const TGeoMaterial* pMaterial)
//BeamlineObject::BeamlineObject(const TGeoMixture* pMaterial)
{
  //std::cout << " beamline material: "<< pMaterial->GetName() << std::endl;
  nucIntLen = pMaterial->GetIntLen();
  density = pMaterial->GetDensity();
 

  TGeoMixture* pMaterialmix;
  pMaterialmix = (TGeoMixture*)pMaterial;
  // unsigned int nElements = pMaterialmix->GetNelements(); 
  // const double* Z_array = pMaterialmix->GetZmixt();//array of Z of the elements
  // //const double* A_array = pMaterialmix->GetAmixt();//array of A of the elements
  // const double* W_array = pMaterialmix->GetWmixt();//array of relative proportions by mass
  // //const double* N_array = pMaterialmix->GetNmixt();//array of numbers atoms
  //std::cout<< "beamline: "<< pMaterialmix->GetName() << " no. of elements: "<< pMaterialmix->GetNelements()<<std::endl;
 

  Z = 0.; A = 0.; N = 0.;
  
  // double Avogardo = 6.02214199e+23;
  
  // pMaterialmix->Print(" ");  
  // std::cout<<" From direct TMaterial  A:"<<pMaterial->GetA()<<" Total Z: "<<pMaterial->GetZ()<<std::endl;
 
  Z=pMaterial->GetZ();
  A=pMaterial->GetA();
  N=A-Z;
  
  // protonPerc = Z/N;  //N here for now stands for total nucleons
  // Z = A*protonPerc;
  // N = A - Z;
}

bool BeamlineObject::operator < (const BeamlineObject& obj) const
{
    return z0 < obj.z0;
}

std::ostream& operator << (std::ostream& os, const BeamlineObject& obj)
{
  
 os << "Beamline object name " << obj.name << " at " << obj.z_up << " <-- " << obj.z0 << " --> " << obj.z_down << "\n"
       << "   Z = " << obj.Z << ", A = " << obj.A << ", N = " << obj.N << "\n"
       << "   Nuclear inc. len. = " << obj.nucIntLen << ", density = " << obj.density << "\n"
       << "   " << obj.length/obj.nucIntLen*100. << "% interaction length, " << " upstream attenuation = " << 1. - obj.attenuation/obj.attenuationSelf << "\n"
       << "   Attenuation by itself = " << obj.attenuationSelf << ", " << " real attenuation = " << obj.attenuation << "\n"
       << "   Collision prob = " << obj.prob << ", accumulated prob = " << obj.accumulatedProb;
 return os;

}

double BeamlineObject::getZ()
{
  time_t time0 = time(NULL);
  // time_t time1 = time(NULL);
  Int_t seed = (Int_t)(time0);

  TRandom3* random = new TRandom3(seed);
  return z_up - nucIntLen*TMath::Log(1. - attenuationSelf*random->Uniform(0,1)); //Todo Fix this place holders
  std::cout<<" random: "<<random->Uniform(0,1)<<std::endl;
}

bool BeamlineObject::inAcceptance(double x, double y)
{
  return true;
  //return x*x/1.9/1.9 + y*y/2.1/2.1 < 1.;
}
