#include "E906VertexGen.h"
#include <iostream>
#include "BeamlineObject.h"
#include <phgeom/PHGeomUtility.h>
#include <TGeoTube.h>
#include <TGeoManager.h>
#include <TGeoBBox.h>
#include <TRandom3.h>
#include <TMath.h>
#include <TFile.h>


E906VertexGen::E906VertexGen() 
{
inited = false;

}
E906VertexGen::~E906VertexGen(){}

void E906VertexGen::Initfile(){
 
}

void E906VertexGen::InitRun(PHCompositeNode* topNode)
{
  beam_profile= false;
  beamProfile = NULL;
  beamProfile = new TF2("beamProfile", "exp(-0.5*(x-[0])*(x-[0])/[1]/[1])*exp(-0.5*(y-[2])*(y-[2])/[3]/[3])", -10., 10., -10., 10.);
  beamProfile->SetParameter(0, 0.0);
  beamProfile->SetParameter(1, 0.68);
  beamProfile->SetParameter(2, 0.0);
  beamProfile->SetParameter(3, 0.76);

  //beamProfile = new TF2("beamProfile", "exp(-0.5*(x-0.)*(x-0.)/0.68/0.68)*exp(-0.5*(y-0.)*(y-0.)/0.76/0.76)", -10., 10., -10., 10.);
  
  nPieces=0;
 
}

void E906VertexGen::traverse(TGeoNode* node,  double&xvertex,double&yvertex,double&zvertex) 
{
  if(node == NULL) return;

   //Generate perpendicular vtx by sampling beam profile or random guassian 
  double x=0.; 
  double y=0.;

  generateVtxPerp(x, y);


 
  for(int i = 0; i < node->GetNdaughters(); ++i) // Loop over daughter volumes
    {
     
	TGeoVolume* pv= node->GetDaughter(i)->GetVolume();
	TGeoMatrix* mat= node->GetDaughter(i)->GetMatrix();
	const Double_t* pos= mat->GetTranslation();
	
   	TGeoVolumeMulti* pv_multi;
	pv_multi = (TGeoVolumeMulti*)pv;	

	TGeoShape *shape = pv->GetShape();
	TGeoTube* tube;
	TGeoBBox* box;
	tube= (TGeoTube*)shape;
	box=(TGeoBBox*)shape;
    
	// Select those volumes in beamline; not beyond FMAG and not ahead of collimator
	if(fabs(pos[0]) > 1. || fabs(pos[1]) > 7.|| pos[2]>500. || pos[2]<-800.) continue;

	if (node->GetDaughter(i)->GetNdaughters()==0){ // Condition for the volumes with zero subvolumes starts

	TString name = pv->GetName();
	BeamlineObject newObj= BeamlineObject(pv->GetMaterial());

	newObj.name = name;
	newObj.z0 = pos[2];  

	if(name.Contains("Shielding"))
	  {
	    newObj.length = 2.*box->GetDZ();
	    newObj.z_down = newObj.z0 + 0.5*newObj.length;
	    newObj.z_up = newObj.z0 - 0.5*newObj.length;

	    if (x*x+y*y<10.*10.){//Manually get a hole of 10 cm for all shieldings 
	      	newObj.nucIntLen = 65932.038;
	    	newObj.density = 0.001;
	    	newObj.A = 14.364;
	    	newObj.Z = 7.179;
	    	newObj.N =  newObj.A-newObj.Z;
	    }
	  }
	else // only gets target
	  {	  
	    newObj.length = 2.*tube->GetDZ();
	    newObj.z_down = newObj.z0 + 0.5*newObj.length;
	    newObj.z_up = newObj.z0 - 0.5*newObj.length;
	    newObj.radiusX = tube->GetRmax();//outer radius
	    newObj.radiusY = newObj.radiusX;

	     if (x*x+y*y>1.*1.){//to make sure target is not contributing beyond its dimenstion (20 mm diameter)
	      	newObj.nucIntLen = 65932.038;
	    	newObj.density = 0.001;
	    	newObj.A = 14.364;
	    	newObj.Z = 7.179;
	    	newObj.N =  newObj.A-newObj.Z;
	     }
	    
	  }
	
	interactables.push_back(newObj);
      
	}//Condition for the volumes with zero subvolumes ends

	if(node->GetDaughter(i)->GetNdaughters()>0){ //Condition for volumes with multiple subvolumes starts
	  
	  for(int j=0; j<node->GetDaughter(i)->GetNdaughters();++j){ // Loop for the subvolumes Starts
	  
	    TGeoVolume* pv1= node->GetDaughter(i)->GetDaughter(j)->GetVolume();
	    TGeoMatrix* mat1= node->GetDaughter(i)->GetDaughter(j)->GetMatrix();
	    const Double_t* pos1= mat1->GetTranslation();
	    
	    TGeoShape *shape1 = pv1->GetShape();
	    TGeoTube* tube1;
	    TGeoBBox* box1;
	    tube1= (TGeoTube*)shape1;
	    box1=(TGeoBBox*)shape1;

	  
	    // choose those volumes in beamlines 
	    if(fabs(pos1[0]+pos[0]) > 1. || fabs(pos1[1]+pos[1]) > 1.|| (pos1[2]+pos[2])>500.) continue;
	
	    TString name = pv1->GetName();
	    BeamlineObject newObj1= BeamlineObject(pv1->GetMaterial());

	    newObj1.name = name;
	    newObj1.z0 = pos1[2]+pos[2]; 
	    
	    if(name.Contains("fmag"))
	      {
      	     
		newObj1.length = 2.*box1->GetDZ();
		newObj1.z_down = newObj1.z0 + 0.5*newObj1.length;
		newObj1.z_up = newObj1.z0 - 0.5*newObj1.length + 10.*2.54;// Drilling a hole with air upto 25 cm
		newObj1.length = newObj1.length - 10.*2.54;
		newObj1.radiusX = 500.;
		newObj1.radiusY = 500.;

		//Manually hard coded by Abi for Iron
		newObj1.nucIntLen = 16.77;
		newObj1.density = 7.87;
		newObj1.A = 55.845;
		newObj1.Z = 26;
		newObj1.N =  newObj1.A-newObj1.Z;
	      }
	    
	    else{//gets the  collimeter (depending upon the upstream cut)
	      
	      	 newObj1.length = 2.*box1->GetDZ();
		 newObj1.z_down = newObj1.z0 + 0.5*newObj1.length;
		 newObj1.z_up = newObj1.z0 - 0.5*newObj1.length;
		
		 if (fabs(x)<7.8232 && fabs(y)<3.4798){//Manually get a rectangular hole
		   newObj1.nucIntLen = 65932.038;
		   newObj1.density = 0.001;
		   newObj1.A = 14.364;
		   newObj1.Z = 7.179;
		   newObj1.N =  newObj1.A-newObj1.Z;
		 }
	
		 // continue;
	    }
      	  interactables.push_back(newObj1);
	  } // Loop for the subvolumes Ends

      std::sort(interactables.begin(), interactables.end());

	} //Condition for volumes with multiple subvolumes Ends
    
    }

  //sort the vectors by position
  std::sort(interactables.begin(), interactables.end());

 
  //===Fill the gaps betwen the volumes with air
  TGeoMaterial* air = new TGeoMaterial("Air");
  BeamlineObject airgap(air);
  int nGaps = 0;
  unsigned int n_temp = interactables.size();
   for(unsigned int i = 1; i < n_temp; ++i)
     {
       if(fabs(interactables[i-1].z_down - interactables[i].z_up) < 0.1) continue;

       airgap.z_up = interactables[i-1].z_down;
       airgap.z_down = interactables[i].z_up;
       airgap.z0 = 0.5*(airgap.z_up + airgap.z_down);
       airgap.length = airgap.z_down - airgap.z_up;
       airgap.name = Form("AirGap_%d", nGaps++);
       airgap.A = 14.364;
       airgap.Z= 7.179;
       airgap.N= airgap.A- airgap.Z;
       airgap.nucIntLen = 65932.038;
       airgap.density=0.001; 

       interactables.push_back(airgap);
     }
   std::sort(interactables.begin(), interactables.end());


   //===set the quanties that rely on its neighbours
   double attenuationSum = 0.;
   for(unsigned int i = 0; i < interactables.size(); ++i)
     {
      interactables[i].attenuationSelf = 1. - TMath::Exp(-interactables[i].length/interactables[i].nucIntLen);
      interactables[i].attenuation = (1. - attenuationSum)*interactables[i].attenuationSelf;
      interactables[i].prob = interactables[i].attenuation*interactables[i].density*interactables[i].nucIntLen;

      attenuationSum += interactables[i].attenuation;
       
    }

  //===set the accumulatedProbs
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
    

  //===Normalize the accumulated probability
  for(int i = 0; i < nPieces+1; ++i)
    {
    accumulatedProbs[i] = accumulatedProbs[i]/accumulatedProbs[nPieces];
  
    }
 
  // for(int i = 0; i < nPieces; ++i)
  //   {
  //     std::cout << i << " " << interactables[i] << std::endl;
  //     std::cout<<"=====Accumulated Probs=="<<accumulatedProbs[i]<<std::endl;
  //   }
 
  //===Generate vertices and store them 
 double  vtx = generateVertex();
  xvertex= x;
  yvertex= y;
  zvertex= vtx;
 
  interactables.clear();  //clear the vector in order to run the multiple events ; Abi
  
}


//==============
 double E906VertexGen::generateVertex()
{ 
  
  findInteractingPiece();

  //Generate z-vtx
  double zOffset =0.;
  double z = interactables[index].getZ() + zOffset;

  return z;
}

void E906VertexGen::generateVtxPerp(double& x, double& y)
{
  if(beam_profile)
  {
       beamProfile->GetRandom2(x, y);
    }
    else
    {
      x=gRandom->Gaus(0.,1.5);
      y=gRandom->Gaus(0.,1.5);
    }
}

void E906VertexGen::findInteractingPiece()
{
  //randomly find the index based on their accum probs
  index = TMath::BinarySearch(nPieces+1, accumulatedProbs,gRandom->Uniform(0,1));
 
}

