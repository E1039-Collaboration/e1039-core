#include "DPTriggerAnalyzer.h"


#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <geom_svc/GeomSvc.h>

#include <fun4all/Fun4AllBase.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

//#include <g4main/PHG4TruthInfoContainer.h>
//#include <g4main/PHG4HitContainer.h>
//#include <g4main/PHG4Hit.h>
//#include <g4main/PHG4Hitv1.h>
//#include <g4main/PHG4Particle.h>
//#include <g4main/PHG4HitDefs.h>
//#include <g4main/PHG4VtxPoint.h>

#include <iomanip>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>

#ifndef __CINT__
#include <boost/lexical_cast.hpp>
#endif

using namespace std;

#define IMAX 1000000
#define JMAX 16
#define KMAX 1000
#define LMAX 4

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

int DPTriggerAnalyzer::Init(PHCompositeNode *topNode) 
{
  
 return Fun4AllReturnCodes::EVENT_OK;
}

DPTriggerAnalyzer::DPTriggerAnalyzer(const std::string& name) :
  SubsysReco(name),
  _road_set_file_name("trigger_67.txt"),
  _hit_container_type("Vector"),
  _event(0),
  _run_header(nullptr),
  _spill_map(nullptr),
  _event_header(nullptr),
  _hit_map(nullptr),
  _hit_vector(nullptr),
  p_geomSvc(nullptr)
{
}

DPTriggerAnalyzer::~DPTriggerAnalyzer()
{
  deleteMatrix(matrix[0]);
  deleteMatrix(matrix[1]);
}

int DPTriggerAnalyzer::InitRun(PHCompositeNode* topNode) {
  p_geomSvc = GeomSvc::instance();

  //Load the trigger roads
  int charge = (int) -1e3, roadID = -1;
  int uIDs[NTRPLANES] = { -1, -1, -1, -1 };
  double pXmin = -1.e6, sigWeight = -1.e6, bkgRate = -1.e6;
  string line = "";
  std::ifstream fin(_road_set_file_name.c_str(), std::ifstream::in);
  if (fin.is_open()) {
    while (getline(fin, line)) {
      stringstream ss(line);

      ss >> charge >> roadID;
      for (int i = 0; i < NTRPLANES; ++i)
        ss >> uIDs[i];
      ss >> pXmin >> sigWeight >> bkgRate;

      DPTriggerRoad road;
      road.setRoadID(roadID);
      road.setSigWeight(sigWeight);
      road.setBkgRate(bkgRate);
      road.setPxMin(pXmin);

      for (int i = 0; i < NTRPLANES; ++i)
        road.addTrElement(uIDs[i]);
      roads[(-charge + 1) / 2].insert(
          std::map<TString, DPTriggerRoad>::value_type(road.getStringID(),
              road));
    }
  } else {
    NIMONLY = true;
    LogInfo("NIM Only mode NOT supported now.");
    return Fun4AllReturnCodes::ABORTRUN;
  }
  //build the search matrix
  buildTriggerMatrix();

  int ret = MakeNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK)
    return ret;

  ret = GetNodes(topNode);
  if (ret != Fun4AllReturnCodes::EVENT_OK)
    return ret;

  return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::process_event(PHCompositeNode* topNode) {

  if(Verbosity() >= Fun4AllBase::VERBOSITY_EVEN_MORE)
    std::cout << "Entering DPTriggerAnalyzer::process_event: " << _event << std::endl;

  _event_header->Reset();

  if(_spill_map) {
    auto spill_info=_spill_map->get(spill_id);
    if(spill_info) {
      target_pos=spill_info->get_target_pos();
    } else {
      LogInfo("");
    }
  }

  if(_event_header) {
    _event_header->set_event_id(_event);
    event_id=_event_header->get_event_id();
    spill_id=_event_header->get_spill_id();
    run_id=_event_header->get_run_id();
  }
    
  //NIM trigger first
  // e1039 fDetectorID : 1-6, 7-12, 13-18, 19-24, 25-30 for ChamberAcc in ST 1(UU'/XX'/VV'), 2(UU'/XX'/VV'), 3+(UU'/XX'/VV'), 3-(UU'/XX'/VV'), optional chamber (Colorado?)
  // e1039 fDetectorID : 31-34, 35-38, 39-40, 41-46 for HodoAcc in ST 1(H1XT/B,H1YL/R), 2(H2XT/B,H2YL/R), 3(H3XT/B), 4(H4XT/B,H4Y1L/R,H4Y2L/R)
  // e1039 fDetectorID : 47-54 in ST4(H1X/Y,V1X/Y,H2X/Y,V2X/Y)
  // Kun's DPSim : 25/26, 31/32, 33/34, 39/40 in ST 1(XB/XT), 2(XB/XT), 3(XB/XT), 4(XB/XT)
  // Kun's DPSim : 27/28, 29/30, 35/36, 37/38 in ST 1(YL/YR), 2(YL/YR), 3(YL/YR), 4(YL/YR)

  Nhits_YNIM_1XT=0;
  Nhits_YNIM_2XT=0;
  Nhits_YNIM_3XT=0;
  Nhits_YNIM_4XT=0;
  Nhits_YNIM_1XB=0;
  Nhits_YNIM_2XB=0;
  Nhits_YNIM_3XB=0;
  Nhits_YNIM_4XB=0;
  Nhits_YNIM_1YL=0;
  Nhits_YNIM_2YL=0;
  Nhits_YNIM_4Y1L=0;
  Nhits_YNIM_4Y2L=0;
  Nhits_YNIM_1YR=0;
  Nhits_YNIM_2YR=0;
  Nhits_YNIM_4Y1R=0;
  Nhits_YNIM_4Y2R=0;

  if (_hit_vector) {
    for (Int_t ihit = 0; ihit < _hit_vector->size(); ++ihit) {
      SQHit *hit = _hit_vector->at(ihit);

      if (!hit) {
        if (Verbosity() >= Fun4AllBase::VERBOSITY_MORE) {
          LogInfo("!hit");
        }
      }

      if(p_geomSvc->getTriggerLv(hit->get_detector_id())<0) continue;

      if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
        std::cout
        << " detector_id: " << hit->get_detector_id()
        << " TriggerLv: " << p_geomSvc->getTriggerLv(hit->get_detector_id())
        << std::endl;
      }

      if(hit->get_detector_id()<31 || hit->get_detector_id()>46) continue;
      if(hit->get_detector_id()==31) Nhits_YNIM_1XB++;
      if(hit->get_detector_id()==32) Nhits_YNIM_1XT++;
      if(hit->get_detector_id()==33) Nhits_YNIM_1YL++;
      if(hit->get_detector_id()==34) Nhits_YNIM_1YR++;
      if(hit->get_detector_id()==37) Nhits_YNIM_2XB++;
      if(hit->get_detector_id()==38) Nhits_YNIM_2XT++;
      if(hit->get_detector_id()==35) Nhits_YNIM_2YL++;
      if(hit->get_detector_id()==36) Nhits_YNIM_2YR++;
      if(hit->get_detector_id()==39) Nhits_YNIM_3XB++;
      if(hit->get_detector_id()==40) Nhits_YNIM_3XT++;
      if(hit->get_detector_id()==45) Nhits_YNIM_4XB++;
      if(hit->get_detector_id()==46) Nhits_YNIM_4XT++;
      if(hit->get_detector_id()==41) Nhits_YNIM_4Y1L++;
      if(hit->get_detector_id()==42) Nhits_YNIM_4Y1R++;
      if(hit->get_detector_id()==43) Nhits_YNIM_4Y2L++;
      if(hit->get_detector_id()==44) Nhits_YNIM_4Y2R++;
    }
  }

  HXB=(Nhits_YNIM_1XB>0 and Nhits_YNIM_2XB>0 and Nhits_YNIM_3XB>0 and Nhits_YNIM_4XB>0);
  HXT=(Nhits_YNIM_1XT>0 and Nhits_YNIM_2XT>0 and Nhits_YNIM_3XT>0 and Nhits_YNIM_4XT>0);
  HYL=(Nhits_YNIM_1YL>0 and Nhits_YNIM_2YL>0 and Nhits_YNIM_4Y1L>0 and Nhits_YNIM_4Y2L>0);
  HYR=(Nhits_YNIM_1YR>0 and Nhits_YNIM_2YR>0 and Nhits_YNIM_4Y1R>0 and Nhits_YNIM_4Y2R>0);

  if(HYL or HYR) _event_header->set_trigger(SQEvent::NIM1, true);
  if(HXT or HXB) _event_header->set_trigger(SQEvent::NIM2, true);

  //For FPGA trigger, build the internal hit pattern first
  if(_hit_vector) {

    n_FPGA_hits=0;
    int nHitsAll=_hit_vector->size();

    for(Int_t ihit = 0; ihit < nHitsAll; ++ihit) {
      SQHit *hit=_hit_vector->at(ihit);
      if(p_geomSvc->getTriggerLv(hit->get_detector_id()) < 0) continue;
      uniqueIDs[n_FPGA_hits++] = hit->get_detector_id()*1000 + hit->get_element_id();
    }

    bool all_planes_have_hits = buildHitPattern(n_FPGA_hits, uniqueIDs);

    //do the tree DFS search
    for(int i = 0; i < 2; ++i) {
      path.clear();
      roads_found[i].clear();
      if(all_planes_have_hits)
        searchMatrix(matrix[i], 0, i);
    }

    //FPGA singles trigger
    nPlusTop=0;
    nPlusBot=0;
    nMinusTop=0;
    nMinusBot=0;
    nHiPxPlusTop=0;
    nHiPxPlusBot=0;
    nHiPxMinusTop=0;
    nHiPxMinusBot=0;

    for (std::list<DPTriggerRoad>::iterator iter = roads_found[0].begin();
        iter != roads_found[0].end(); ++iter) {
      if (iter->getTB() > 0) {
        ++nPlusTop;
        if (iter->getPxMin() > 3.)
          ++nHiPxPlusTop;
      } else {
        ++nPlusBot;
        if (iter->getPxMin() > 3.)
          ++nHiPxPlusBot;
      }
    }

    for (std::list<DPTriggerRoad>::iterator iter = roads_found[1].begin();
        iter != roads_found[1].end(); ++iter) {
      if (iter->getTB() > 0) {
        ++nMinusTop;
        if (iter->getPxMin() > 3.)
          ++nHiPxMinusTop;
      } else {
        ++nMinusBot;
        if (iter->getPxMin() > 3.)
          ++nHiPxMinusBot;
      }
    }
  }

  if((nPlusTop > 0 && nMinusBot > 0) || (nPlusBot > 0 && nMinusTop > 0)) _event_header->set_trigger(SQEvent::MATRIX1, true);
  if((nPlusTop > 0 && nMinusTop > 0) || (nPlusBot > 0 && nMinusBot > 0)) _event_header->set_trigger(SQEvent::MATRIX2, true);
  if((nPlusTop > 0 && nPlusBot > 0) || (nMinusTop > 0 && nMinusBot > 0)) _event_header->set_trigger(SQEvent::MATRIX3, true);
  if(nPlusTop > 0 || nMinusTop > 0 || nPlusBot > 0 || nMinusBot > 0) _event_header->set_trigger(SQEvent::MATRIX4, true);
  if(nHiPxPlusTop > 0 || nHiPxMinusTop > 0 || nHiPxPlusBot > 0 || nHiPxMinusBot > 0) _event_header->set_trigger(SQEvent::MATRIX5, true);

  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) {
    _event_header->identify();
  }

  _event++;

  if(Verbosity() >= Fun4AllBase::VERBOSITY_EVEN_MORE)
    std::cout << "Leaving DPTriggerAnalyzer::process_event: " << _event << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;

}

int DPTriggerAnalyzer::ResetEvalVars() {
  run_id = std::numeric_limits<int>::max();
  spill_id = std::numeric_limits<int>::max();
  target_pos = std::numeric_limits<float>::max();
  event_id = std::numeric_limits<int>::max();

  return 0;
}

int DPTriggerAnalyzer::End(PHCompositeNode* topNode) {
  return Fun4AllReturnCodes::EVENT_OK;
}


int DPTriggerAnalyzer::MakeNodes(PHCompositeNode* topNode) {

  PHNodeIterator iter(topNode);

  PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (!eventNode) {
    LogInfo("No DST node, create one");
    eventNode = new PHCompositeNode("DST");
    topNode->addNode(eventNode);
  }

  _event_header = new SQEvent_v1();
  PHIODataNode<PHObject>* eventHeaderNode = new PHIODataNode<PHObject>(_event_header,"SQEvent", "PHObject");
  eventNode->addNode(eventHeaderNode);
  if (verbosity >= Fun4AllBase::VERBOSITY_SOME)
    LogInfo("DST/SQEvent Added");

  return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::GetNodes(PHCompositeNode* topNode) {
  
  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogInfo("!_event_header");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  if(_hit_container_type.find("Map") != std::string::npos) {
    _hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
    if (!_hit_map) {
      LogInfo("!_hit_map");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  if(_hit_container_type.find("Vector") != std::string::npos) {
    _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
    if (!_hit_vector) {
      LogInfo("!_hit_vector");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void DPTriggerAnalyzer::buildTriggerMatrix() {
  for (int i = 0; i < 2; ++i) {
    matrix[i] = new MatrixNode(-1);
    for (std::map<TString, DPTriggerRoad>::iterator iter = roads[i].begin(); iter != roads[i].end(); ++iter) {
      MatrixNode* parentNode[NTRPLANES + 1]; //NOTE: the last entry is useless, just to keep the following code simpler
      parentNode[0] = matrix[i];
      for (int j = 0; j < NTRPLANES; ++j) {
        int uniqueID = iter->second.getTrID(j);
        bool isNewNode = true;
        for (std::list<MatrixNode*>::iterator jter = parentNode[j]->children.begin(); jter != parentNode[j]->children.end(); ++jter) {
          if (uniqueID == (*jter)->uniqueID) {
            parentNode[j + 1] = *jter;
            isNewNode = false;

            break;
          }
        }

        if (isNewNode) {
          MatrixNode* newNode = new MatrixNode(uniqueID);
          parentNode[j]->add(newNode);
          parentNode[j + 1] = newNode;
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
  for(int i = 0; i < nTrHits; ++i){
    int detectorID = uniqueTrIDs[i]/1000;
    int index = p_geomSvc->getTriggerLv(detectorID);
    if(index < 0 || index >= NTRPLANES) continue;

    trHits[index].insert(uniqueTrIDs[i]);
  }
  
  for (int i = 0; i < NTRPLANES; ++i) {
    if (trHits[i].empty())
      return false;
    data.push_back(trHits[i]);
  }
  
  return true;
}

void DPTriggerAnalyzer::searchMatrix(MatrixNode* node, int level, int index) {
  path.push_back(node->uniqueID);
  if (node->children.empty()) {
    //printPath();
    DPTriggerRoad road_found(path);
    if (roads[index].find(road_found.getStringID()) != roads[index].end())
      roads_found[index].push_back(road_found);
    path.pop_back();

    return;
  }

  for (std::list<MatrixNode*>::iterator iter = node->children.begin(); iter != node->children.end(); ++iter) {
    if (data[level + 1].find((*iter)->uniqueID) == data[level + 1].end())
      continue;
    searchMatrix(*iter, level + 1, index);
  }
  path.pop_back();
}

void DPTriggerAnalyzer::deleteMatrix(MatrixNode* node) {
  if (node == NULL)
    return;

  if (node->children.empty()) {
    delete node;
    return;
  }

  for (std::list<MatrixNode*>::iterator iter = node->children.begin(); iter != node->children.end(); ++iter) {
    deleteMatrix(*iter);
  }

  delete node;
}

void DPTriggerAnalyzer::printHitPattern() {
  for (unsigned int i = 1; i < NTRPLANES; ++i) {
    std::cout << "Lv. " << i << ":  ";
    for (std::set<int>::iterator iter = data[i].begin(); iter != data[i].end(); ++iter) {
      std::cout << *iter << "  ";
    }
    std::cout << std::endl;
  }
}

void DPTriggerAnalyzer::printPath() {
  std::cout << "Found one road: " << std::endl;
  for (std::list<int>::iterator iter = path.begin(); iter != path.end(); ++iter) {
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
