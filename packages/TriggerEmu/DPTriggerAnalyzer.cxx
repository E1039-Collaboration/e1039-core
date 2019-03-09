#include "DPTriggerAnalyzer.h"
#include "DPDigitizer.h"

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Hitv1.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4HitDefs.h>
#include <g4main/PHG4VtxPoint.h>

#include <fstream>
#include <sstream>
#include <string>

#ifndef __CINT__
#include <boost/lexical_cast.hpp>
#endif

using namespace std;

DPTriggerRoad::DPTriggerRoad() : roadID(0), sigWeight(0.), bkgRate(0.), pXmin(0.)
{
  uniqueTrIDs.clear();
}

DPTriggerRoad::DPTriggerRoad(const std::list<int>& path) : roadID(0), sigWeight(0.), bkgRate(0.), pXmin(0.)
{
  uniqueTrIDs.clear();
  for(std::list<int>::const_iterator iter = path.begin(); iter != path.end(); ++iter)
    {
      if(*iter < 0) continue;
      uniqueTrIDs.push_back(*iter);
    }
}

int DPTriggerRoad::getTB()
{
  int TB = 0;
  for(unsigned int i = 0; i < NTRPLANES; ++i)
    {
      int detectorID = getTrDetectorID(i);
      TB = TB + (((detectorID & 1) == 0) ? 1 : -1);
    }
  
  if(TB == 4) return 1;
  if(TB == -4) return -1;
  return 0;
}

void DPTriggerRoad::flipTB()
{
  int corr = getTB();
  for(unsigned int i = 0; i < NTRPLANES; ++i)
    {
      int detectorID = getTrDetectorID(i);
      int elementID = getTrElementID(i);
      
      detectorID -= corr;
      uniqueTrIDs[i] = detectorID*1000 + elementID;
    }
}

bool DPTriggerRoad::operator == (const DPTriggerRoad& elem) const
{
  for(unsigned int i = 0; i < NTRPLANES; ++i)
    {
      if(uniqueTrIDs[i] != elem.uniqueTrIDs[i]) return false;
    }
  
  return true;
}

bool DPTriggerRoad::operator < (const DPTriggerRoad& elem) const
{
  return sigWeight < elem.sigWeight;
}

TString DPTriggerRoad::getStringID()
{
  TString sid;
  for(unsigned int i = 0; i < uniqueTrIDs.size(); ++i)
    {
      sid = sid + Form("%06d", uniqueTrIDs[i]);
    }
  
  return sid;
}

std::ostream& operator << (std::ostream& os, const DPTriggerRoad& road)
{
  os << "Trigger Road ID = " << road.roadID << ", signal = " << road.sigWeight << ", bkg = " << road.bkgRate << "\n   ";
  for(unsigned int i = 0; i < NTRPLANES; ++i)
    {
      os << road.uniqueTrIDs[i] << " == ";
    }
  
  return os;
}

//DPTriggerAnalyzer* DPTriggerAnalyzer::p_triggerAna = NULL;
//DPTriggerAnalyzer* DPTriggerAnalyzer::instance()
//{
//    if(p_triggerAna == NULL) p_triggerAna = new DPTriggerAnalyzer;
//    return p_triggerAna;
//}

int DPTriggerAnalyzer::Init(PHCompositeNode *topNode) 
{

  return Fun4AllReturnCodes::EVENT_OK;
  
}

int DPTriggerAnalyzer::InitRun(PHCompositeNode* topNode) : 
  NIMONLY(false) 
{

	p_geomSvc = GeomSvc::instance();

	if(!p_geomSvc->checkFile(p_geomSvc->triggerMatrix))
	  {
	    NIMONLY = true;
	    return;
	  }

	//Load the trigger roads
	ifstream fin(p_geomSvc->triggerMatrix.Data()); 
	if(fin) {
	  string line;
	  while(getline(fin, line))
	    {
	      stringstream ss(line);
	      
	      int charge, roadID;
	      int uIDs[NTRPLANES];
	      double pXmin, sigWeight, bkgRate;
      
	      ss >> charge >> roadID;
	      for(int i = 0; i < NTRPLANES; ++i) ss >> uIDs[i];
	      ss >> pXmin >> sigWeight >> bkgRate;
      
	      DPTriggerRoad road;
	      road.setRoadID(roadID);
	      road.setSigWeight(sigWeight);
	      road.setBkgRate(bkgRate);
	      road.setPxMin(pXmin);
      
	      for(int i = 0; i < NTRPLANES; ++i) road.addTrElement(uIDs[i]);
	      roads[(-charge+1)/2].insert(std::map<TString, DPTriggerRoad>::value_type(road.getStringID(), road));
	    }
	}

	//build the search matrix
	buildTriggerMatrix();

	int ret = GetNodes(topNode);
	if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

	return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::process_event(PHCompositeNode* topNode) {

  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME)
    std::cout << "Entering TrkEval::process_event: " << _event << std::endl;

  
}

//DPTriggerAnalyzer::DPTriggerAnalyzer() : NIMONLY(false)
DPTriggerAnalyzer::DPTriggerAnalyzer(const std::string &name, const int verbo) : 
  SubsysReco(name),
  p_geomSvc(nullptr),
  _run_header(nullptr),
  _spill_map(nullptr),
  _event_header(nullptr),
  _hit_map(nullptr),
  _hit_vector(nullptr)
{
  Verbosity(verbo);
  //    DPSimConfig* p_config = DPSimConfig::instance();
  //    if(!p_config->checkFile(p_config->triggerMatrix))
  //    {
  //        NIMONLY = true;
  //        return;
  //    }
  //  p_digitizer = DPDigitizer::instance();

  //load the trigger roads
  //using namespace std;

  //string line;
  //ifstream fin(p_config->triggerMatrix.Data());  assert(fin && "Trigger matrix input not found!");
  //while(getline(fin, line))
  //{
  //stringstream ss(line);
      
  //int charge, roadID;
  //int uIDs[NTRPLANES];
  //double pXmin, sigWeight, bkgRate;

  //ss >> charge >> roadID;
  //for(int i = 0; i < NTRPLANES; ++i) ss >> uIDs[i];
  //ss >> pXmin >> sigWeight >> bkgRate;

  //DPTriggerRoad road;
  //road.setRoadID(roadID);
  //road.setSigWeight(sigWeight);
  //road.setBkgRate(bkgRate);
  //road.setPxMin(pXmin);

  //for(int i = 0; i < NTRPLANES; ++i) road.addTrElement(uIDs[i]);
  //roads[(-charge+1)/2].insert(std::map<TString, DPTriggerRoad>::value_type(road.getStringID(), road));
  //}

  //build the search matrix
  //buildTriggerMatrix();
}

DPTriggerAnalyzer::~DPTriggerAnalyzer()
{
  deleteMatrix(matrix[0]);
  deleteMatrix(matrix[1]);
}

//void DPTriggerAnalyzer::analyzeTrigger(DPMCRawEvent* rawEvent)
void DPTriggerAnalyzer::analyzeTrigger(DPMCRawEvent* rawEvent)
{
  //analyze the roads found and set the trigger bit
  //rawEvent->eventHeader().fTriggerBit = 0;
  //for( SQEvent::TriggerMask i=std::numeric_limits<unsigned short>::min();
  //	i<=SQEvent::TriggerMask i=std::numeric_limits<unsigned short>::max(); ++i) {
  //	_event_header->set_trigger(i,false);
	//}
	_event_header->reset();
  
  //NIM trigger first
  bool HXT = (rawEvent->getNHits(26) > 0) && (rawEvent->getNHits(32) > 0) && (rawEvent->getNHits(34) > 0) && (rawEvent->getNHits(40) > 0);
  bool HXB = (rawEvent->getNHits(25) > 0) && (rawEvent->getNHits(31) > 0) && (rawEvent->getNHits(33) > 0) && (rawEvent->getNHits(39) > 0);
  bool HYL = (rawEvent->getNHits(27) > 0) && (rawEvent->getNHits(29) > 0) && (rawEvent->getNHits(35) > 0) && (rawEvent->getNHits(37) > 0);
  bool HYR = (rawEvent->getNHits(28) > 0) && (rawEvent->getNHits(30) > 0) && (rawEvent->getNHits(36) > 0) && (rawEvent->getNHits(38) > 0);
  if(HYL || HYR) rawEvent->eventHeader().fTriggerBit |= NIM1;
  if(HXT || HXB) rawEvent->eventHeader().fTriggerBit |= NIM2;
  
  //if matrix is not inited, return
  if(NIMONLY) return;
  
  //For FPGA trigger, build the internal hit pattern first
  int nHits = 0;
  int uniqueIDs[10000];
  
  int nHitsAll = rawEvent->getNHits();
  TClonesArray* hits = rawEvent->getHits();
  for(int i = 0; i < nHitsAll; ++i)
    {
      DPMCHit* hit = (DPMCHit*)hits->At(i);
      if(p_digitizer->getTriggerLv(hit->fDetectorID) < 0) continue;
      
      uniqueIDs[nHits++] = hit->uniqueID();
    }
  if(!buildHitPattern(nHits, uniqueIDs)) return;
  
  //do the tree DFS search
  for(int i = 0; i < 2; ++i)
    {
      path.clear();
      roads_found[i].clear();
      searchMatrix(matrix[i], 0, i);
    }
  
  //FPGA singles trigger
  int nPlusTop = 0;
  int nPlusBot = 0;
  int nMinusTop = 0;
  int nMinusBot = 0;
  int nHiPxPlusTop = 0;
  int nHiPxPlusBot = 0;
  int nHiPxMinusTop = 0;
  int nHiPxMinusBot = 0;
  
  for(std::list<DPTriggerRoad>::iterator iter = roads_found[0].begin(); iter != roads_found[0].end(); ++iter)
    {
      rawEvent->eventHeader().fPosRoadIDs.push_back(iter->getRoadID());
      if(iter->getTB() > 0)
        {
	  ++nPlusTop;
	  if(iter->getPxMin() > 3.) ++nHiPxPlusTop;
        }
      else
        {
	  ++nPlusBot;
	  if(iter->getPxMin() > 3.) ++nHiPxPlusBot;
        }
    }
  
  for(std::list<DPTriggerRoad>::iterator iter = roads_found[1].begin(); iter != roads_found[1].end(); ++iter)
    {
      rawEvent->eventHeader().fNegRoadIDs.push_back(iter->getRoadID());
      if(iter->getTB() > 0)
        {
	  ++nMinusTop;
	  if(iter->getPxMin() > 3.) ++nHiPxMinusTop;
        }
      else
        {
	  ++nMinusBot;
	  if(iter->getPxMin() > 3.) ++nHiPxMinusBot;
        }
    }
  
  if((nPlusTop > 0 && nMinusBot > 0) || (nPlusBot > 0 && nMinusTop > 0)) rawEvent->eventHeader().fTriggerBit |= MATRIX1;
  if((nPlusTop > 0 && nMinusTop > 0) || (nPlusBot > 0 && nMinusBot > 0)) rawEvent->eventHeader().fTriggerBit |= MATRIX2;
  if((nPlusTop > 0 && nPlusBot > 0) || (nMinusTop > 0 && nMinusBot > 0)) rawEvent->eventHeader().fTriggerBit |= MATRIX3;
  if(nPlusTop > 0 || nMinusTop > 0 || nPlusBot > 0 || nMinusBot > 0) rawEvent->eventHeader().fTriggerBit |= MATRIX4;
  if(nHiPxPlusTop > 0 || nHiPxMinusTop > 0 || nHiPxPlusBot > 0 || nHiPxMinusBot > 0) rawEvent->eventHeader().fTriggerBit |= MATRIX5;
}

int DPTriggerAnalyzer::GetNodes(PHCompositeNode* topNode) {

  _run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!_run_header) {
    LogError("!_run_header");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  _spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
  if (!_spill_map) {
    LogError("!_spill_map");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogError("!_event_header");
    //return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  if(_hit_container_type.find("Map") != std::string::npos) {
    _hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
    if (!_hit_map) {
      LogError("!_hit_map");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  if(_hit_container_type.find("Vector") != std::string::npos) {
    _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
    if (!_hit_vector) {
      LogError("!_hit_vector");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  _truth = findNode::getClass<PHG4TruthInfoContainer>(topNode, "G4TruthInfo");
  if (!_truth) {
    LogError("!_truth");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
  if (!_recEvent) {
    LogError("!_recEvent");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void DPTriggerAnalyzer::buildTriggerMatrix()
{
  for(int i = 0; i < 2; ++i)
    {
      matrix[i] = new MatrixNode(-1);
      for(std::map<TString, DPTriggerRoad>::iterator iter = roads[i].begin(); iter != roads[i].end(); ++iter)
        {
	  MatrixNode* parentNode[NTRPLANES+1]; //NOTE: the last entry is useless, just to keep the following code simpler
	  parentNode[0] = matrix[i];
	  for(int j = 0; j < NTRPLANES; ++j)
            {
	      int uniqueID = iter->second.getTrID(j);
	      bool isNewNode = true;
	      for(std::list<MatrixNode*>::iterator jter = parentNode[j]->children.begin(); jter != parentNode[j]->children.end(); ++jter)
                {
		  if(uniqueID == (*jter)->uniqueID)
                    {
		      parentNode[j+1] = *jter;
		      isNewNode = false;
		      
		      break;
                    }
                }
	      
	      if(isNewNode)
                {
		  MatrixNode* newNode = new MatrixNode(uniqueID);
		  parentNode[j]->add(newNode);
		  parentNode[j+1] = newNode;
                }
            }
        }
    }
}

bool DPTriggerAnalyzer::buildHitPattern(int nTrHits, int uniqueTrIDs[])
{
  data.clear();
  
  //insert a dummy common root node first
  std::set<int> vertex; vertex.insert(-1);
  data.push_back(vertex);
  
  std::set<int> trHits[NTRPLANES];
  for(int i = 0; i < nTrHits; ++i)
    {
      int detectorID = uniqueTrIDs[i]/1000;
      int index = p_digitizer->getTriggerLv(detectorID);
      if(index < 0 || index >= NTRPLANES) continue;
      
      trHits[index].insert(uniqueTrIDs[i]);
    }
  
  for(int i = 0; i < NTRPLANES; ++i)
    {
      if(trHits[i].empty()) return false;
      data.push_back(trHits[i]);
    }
  
  return true;
}

void DPTriggerAnalyzer::searchMatrix(MatrixNode* node, int level, int index)
{
  path.push_back(node->uniqueID);
  if(node->children.empty())
    {
      //printPath();
      DPTriggerRoad road_found(path);
      if(roads[index].find(road_found.getStringID()) != roads[index].end()) roads_found[index].push_back(road_found);
      path.pop_back();
      
      return;
    }
  
  for(std::list<MatrixNode*>::iterator iter = node->children.begin(); iter != node->children.end(); ++iter)
    {
      if(data[level+1].find((*iter)->uniqueID) == data[level+1].end()) continue;
      searchMatrix(*iter, level+1, index);
    }
  path.pop_back();
}

void DPTriggerAnalyzer::deleteMatrix(MatrixNode* node)
{
  if(node == NULL) return;
  
  if(node->children.empty())
    {
      delete node;
      return;
    }
  
  for(std::list<MatrixNode*>::iterator iter = node->children.begin(); iter != node->children.end(); ++iter)
    {
      deleteMatrix(*iter);
    }
  
  delete node;
}

void DPTriggerAnalyzer::printHitPattern()
{
  for(unsigned int i = 1; i < NTRPLANES; ++i)
    {
      std::cout << "Lv. " << i << ":  ";
      for(std::set<int>::iterator iter = data[i].begin(); iter != data[i].end(); ++iter)
        {
	  std::cout << *iter << "  ";
        }
      std::cout << std::endl;
    }
}

void DPTriggerAnalyzer::printPath()
{
  std::cout << "Found one road: " << std::endl;
  for(std::list<int>::iterator iter = path.begin(); iter != path.end(); ++iter)
    {
      std::cout << *iter << " === ";
    }
  std::cout << std::endl;
}

DPTriggerAnalyzer::MatrixNode::MatrixNode(int uID) : uniqueID(uID)
{
  children.clear();
}

void DPTriggerAnalyzer::MatrixNode::add(MatrixNode* child)
{
  children.push_back(child);
}
