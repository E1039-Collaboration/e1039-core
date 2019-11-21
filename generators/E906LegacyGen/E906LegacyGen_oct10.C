#include "E906LegacyGen.h"


#include "E906VertexGen.h"
#include "BeamlineObject.h"
#include <TGeoMaterial.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <TH1F.h>
#include <TFile.h>
#include <TMath.h>
#include <TTree.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <fun4all/PHTFileServer.h>
#include <phgeom/PHGeomUtility.h>
#include <boost/lexical_cast.hpp>
#define LogError(exp)		std::cout<<"ERROR: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

E906LegacyGen::E906LegacyGen(const std::string& name) 
  : SubsysReco("E906LegacyGen")
{
    _vertexGen = new E906VertexGen();
    _out_name = name;
    const TGeoMaterial* pMaterial;

    ResetVars();
    return;
}

E906LegacyGen::~E906LegacyGen()
{
   delete _vertexGen;
    
}

int E906LegacyGen::Init(PHCompositeNode* topNode)
{
 
  file = new TFile(_out_name.c_str(), "RECREATE");
  InitTree(); 
  return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::process_event(PHCompositeNode* topNode)
{
    _vertexGen->InitRun(topNode);
    TGeoManager* geoManager = PHGeomUtility::GetTGeoManager(topNode);
    double x_vtx,y_vtx,z_vtx;
    x_vtx=0.;
    y_vtx=0.;
    z_vtx=0.;
    _vertexGen->traverse(geoManager->GetTopNode(),x_vtx,y_vtx, z_vtx);
    
    truth_vtxx = x_vtx;
    truth_vtxy = y_vtx;
    truth_vtxz = z_vtx;

    truth_tree->Fill();
 
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::ResetEvent(PHCompositeNode* topNode)
{
    return Fun4AllReturnCodes::EVENT_OK;
}

int E906LegacyGen::End(PHCompositeNode* topNode)
{
  file->Write();
  file->Close();
  return Fun4AllReturnCodes::EVENT_OK;
}
void E906LegacyGen::ResetVars()
{
  truth_vtxx= 0.0;
  truth_vtxy=0.0;
  truth_vtxz=0.0;
}

void E906LegacyGen::InitTree()
{
   truth_tree = new TTree("truthtree","a tree with all truth information from generator");
   truth_tree->Branch("truth_vtxx", &truth_vtxx, "truth_vtxx/F");
   truth_tree->Branch("truth_vtxy", &truth_vtxy, "truth_vtxy/F");
   truth_tree->Branch("truth_vtxz", &truth_vtxz, "truth_vtxz/F");
  
}

double E906LegacyGen::getvtxx(){
  return truth_vtxx;
}

void E906LegacyGen::setvtxx(double vtxx){
  truth_vtxx = vtxx;
}
