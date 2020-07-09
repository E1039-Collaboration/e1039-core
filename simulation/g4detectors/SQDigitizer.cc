#include "SQDigitizer.h"

#include <interface_main/SQMCHit_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <g4main/PHG4Hitv1.h>
#include <g4main/PHG4HitContainer.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <geom_svc/GeomSvc.h>

#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/Randomize.hh>

#include <TMath.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>

#ifndef __CINT__
#include <boost/lexical_cast.hpp>
#endif

#ifndef __CINT__
int SQDigitizer::Init(PHCompositeNode *topNode)
{
	p_geomSvc = GeomSvc::instance();
  detIDByName.clear();
  for(int i = 1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes+nDarkPhotonPlanes; ++i)
  {
    if(!enableDC1 && (i >= 7 && i <= 12)) continue;
    if(!enableDPHodo && (i > nChamberPlanes+nHodoPlanes+nPropPlanes && i <= nChamberPlanes+nHodoPlanes+nPropPlanes+nDarkPhotonPlanes)) continue;      

    std::string detName = p_geomSvc->getDetectorName(i);
    detIDByName[detName] = i;
  }

  /*
  //TODO: Load the detector realization setup
  const char* detectorEffResol = "";
  std::ifstream fin(detectorEffResol);
  if(fin) 
  {
    std::string line;
    while(getline(fin, line))
    {
      std::string detectorName;
      int elementID;
      double eff, res;

      std::stringstream ss(line);
      ss >> detectorName >> elementID >> eff >> res;

      if(eff >= 0. && eff <= 1. && res >= 0.)
      {
        digiPlanes[map_detectorID[detectorName]].efficiency[elementID] = eff;
        digiPlanes[map_detectorID[detectorName]].resolution[elementID] = res;
      }
    }
  }
  */

  return Fun4AllReturnCodes::EVENT_OK;
}
#endif

SQDigitizer::SQDigitizer(const std::string& name, const int verbose): 
  SubsysReco(name), 
  p_geomSvc(nullptr),
  enableDC1(false),
  enableDPHodo(true)
{
	Verbosity(0);
}

SQDigitizer::~SQDigitizer() 
{}

int SQDigitizer::InitRun(PHCompositeNode* topNode) 
{
  if(Verbosity() > 2) std::cout << "SQDigitizer: InitRun" << std::endl;
  
  PHNodeIterator iter(topNode);

  // Looking for the DST node
  PHCompositeNode *dstNode;
  dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if(!dstNode)
  {
    std::cerr << Name() << " DST Node missing, abort." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  // Book input g4hits - everything should be present
  hitContainerByName.clear();
  for(auto it = detIDByName.begin(); it != detIDByName.end(); ++it)
  {

    std::string detName = it->first;
    std::string g4hitNodeName = "G4HIT_" + detName;
    if(Verbosity() > 2)
    {
      std::cout << Name() << ": booking input G4HIT node " << g4hitNodeName << std::endl;
    }

    PHG4HitContainer* hits = findNode::getClass<PHG4HitContainer>(topNode, g4hitNodeName.c_str());
    if(!hits)
    {
      std::cerr << Name() << ": Could not locate g4 hit node " << g4hitNodeName << std::endl;
      return Fun4AllReturnCodes::ABORTRUN;
    }
    hitContainerByName[detName] = hits;
  }

  //Book output SQHits
  digits = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if(!digits)
  {
    digits = new SQHitVector_v1();
    PHIODataNode<PHObject>* newNode = new PHIODataNode<PHObject>(digits, "SQHitVector", "PHObject");
    dstNode->addNode(newNode);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQDigitizer::process_event(PHCompositeNode* topNode) 
{
  for(auto it = hitContainerByName.begin(); it != hitContainerByName.end(); ++it)
  {
    if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT)
    {
      std::cout << Name() << ": Digitizing hits for " << it->first << std::endl;
    }

    int detID = detIDByName[it->first];
    if(detID <= nChamberPlanes+nHodoPlanes+nPropPlanes+nDarkPhotonPlanes) 
    {
      digitizePlane(it->first);
    }
    else
    {
      digitizeEMCal(it->first);
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void SQDigitizer::digitizePlane(const std::string& detName)
{
  PHG4HitContainer* g4hits = hitContainerByName[detName];
  if(g4hits->size() < 1) return;

  int detID = detIDByName[detName];
  for(PHG4HitContainer::ConstIterator it = g4hits->getHits().first; it != g4hits->getHits().second; ++it)
  {
    const PHG4Hit& g4hit = *(it->second);

    int track_id = g4hit.get_trkid();
    if(track_id < 0) continue;         //only save primary track hits

    //get average momentum and position
    double x  = 0.5*(g4hit.get_x(0)  + g4hit.get_x(1));
    double y  = 0.5*(g4hit.get_y(0)  + g4hit.get_y(1));
    if(!p_geomSvc->isInPlane(detID, x, y)) continue;  //only save in-acceptance hits

    double z  = 0.5*(g4hit.get_z(0)  + g4hit.get_z(1));
    double px = g4hit.get_px(0);
    double py = g4hit.get_py(0);
    double pz = g4hit.get_pz(0);
    
    double tx = px/pz;
    double ty = py/pz;
    double x0 = x - tx*z;
    double y0 = y - ty*z;

    double w = p_geomSvc->getInterception(detID, tx, ty, x0, y0);
    int eleID = p_geomSvc->getExpElementID(detID, w);
    if(eleID < 1 || eleID > p_geomSvc->getPlaneNElements(detID)) continue; //only save hits within active region

    SQMCHit_v1 digiHit;
    digiHit.set_track_id(track_id);
    digiHit.set_g4hit_id(g4hit.get_hit_id());
    digiHit.set_truth_x(x);
    digiHit.set_truth_y(y);
    digiHit.set_truth_z(z);
    digiHit.set_truth_px(px);
    digiHit.set_truth_py(py);
    digiHit.set_truth_pz(pz);
    digiHit.set_hit_id(digits->size());
    digiHit.set_in_time(1);
    digiHit.set_hodo_mask(0);
    digiHit.set_detector_id(detID);
    digiHit.set_element_id(eleID);
    digiHit.set_tdc_time(0.);
    digiHit.set_pos(p_geomSvc->getMeasurement(detID, eleID));

    //Drift distance is calculated differently for chamber and hodos
    if(detID <= nChamberPlanes || (detID > nChamberPlanes+nHodoPlanes && detID <= nChamberPlanes+nHodoPlanes+nPropPlanes))
    {
      digiHit.set_drift_distance(p_geomSvc->getDCA(detID, eleID, tx, ty, x0, y0));
    }
    else
    {
      digiHit.set_drift_distance(0.);
    }

    //push the hit to vector
    digits->push_back(&digiHit);

    //special treatment for hodoscopes
    double dw = w - digiHit.get_pos();
    double hodoWidth = p_geomSvc->getCellWidth(detID)/2.;
    double hodoOverlap = p_geomSvc->getPlaneOverlap(detID);
    if(fabs(dw) > hodoWidth - hodoOverlap && fabs(dw) < hodoWidth) //hit happens in the overlap region
    {
      if(dw < 0. && eleID != 1)
      {
        digiHit.set_element_id(eleID-1);
        digiHit.set_pos(p_geomSvc->getMeasurement(detID, eleID-1));
        digiHit.set_hit_id(digits->size());

        digits->push_back(&digiHit);
      }
      else if(dw > 0. && eleID != p_geomSvc->getPlaneNElements(detID))
      {
        digiHit.set_element_id(eleID+1);
        digiHit.set_pos(p_geomSvc->getMeasurement(detID, eleID+1));
        digiHit.set_hit_id(digits->size());

        digits->push_back(&digiHit);
      }
    }
  }
}

void SQDigitizer::digitizeEMCal(const std::string& detName)
{}

bool SQDigitizer::realize(SQHit& dHit)
{
  //TODO: implement efficiency and calibration smearing
  return true;
}
