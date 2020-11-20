#include "DPTriggerAnalyzer.h"
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQHitVector.h>
#include <geom_svc/GeomSvc.h>
#include <fun4all/Fun4AllBase.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <TSystem.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
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

////////////////////////////////////////////////////////////////

DPTriggerAnalyzer::DPTriggerAnalyzer(const std::string& name) :
  SubsysReco(name),
  _road_set_file_name("trigger_67.txt"),
  _mode_nim1(NIM_AND),
  _mode_nim2(NIM_AND),
  _run_header(nullptr),
  _event_header(nullptr),
  _hit_vector(nullptr)
{
}

DPTriggerAnalyzer::~DPTriggerAnalyzer()
{
  deleteMatrix(matrix[0]);
  deleteMatrix(matrix[1]);
}

void DPTriggerAnalyzer::set_nim_mode(const NimMode nim1, const NimMode nim2)
{
  _mode_nim1 = nim1;
  _mode_nim2 = nim2;
}

/**
 * A roadset is loaded here, not in InitRun().
 * It is possible but very unlikely that multiple runs taken with different roadsets are analyzed together.
 * When someone moves the contents to InitRun(), please confirm that all variables like "roads" are cleared properly.
 */
int DPTriggerAnalyzer::Init(PHCompositeNode *topNode) 
{
  //Load the trigger roads
  int charge = (int) -1e3, roadID = -1;
  int uIDs[NTRPLANES] = { -1, -1, -1, -1 };
  double pXmin = -1.e6, sigWeight = -1.e6, bkgRate = -1.e6;
  string line = "";
  std::ifstream fin(gSystem->ExpandPathName(_road_set_file_name.c_str()), std::ifstream::in);
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
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::InitRun(PHCompositeNode* topNode)
{
  return GetNodes(topNode);
}

int DPTriggerAnalyzer::process_event(PHCompositeNode* topNode) {
  GeomSvc* geomSvc = GeomSvc::instance();

  //NIM trigger first
  /// Count up the number of hits per hodo planes (and others for simplicity)
  map<int, int> nhit_det; // [det_id] = counts
  for (Int_t ihit = 0; ihit < _hit_vector->size(); ++ihit) {
    nhit_det[ _hit_vector->at(ihit)->get_detector_id() ]++;
    // Note: GeomSvc::getTriggerLv() was used in the past version, but it should not be used since the "trigger level" was not a standard variable.
  }

  bool HXB = nhit_det[ geomSvc->getDetectorID("H1B"  ) ] > 0 && 
             nhit_det[ geomSvc->getDetectorID("H2B"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H3B"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4B"  ) ] > 0   ;
  bool HXT = nhit_det[ geomSvc->getDetectorID("H1T"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H2T"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H3T"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4T"  ) ] > 0   ;
  bool HYL = nhit_det[ geomSvc->getDetectorID("H1L"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H2L"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4Y1L") ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4Y2L") ] > 0   ;
  bool HYR = nhit_det[ geomSvc->getDetectorID("H1R"  ) ] > 0 && 
             nhit_det[ geomSvc->getDetectorID("H2R"  ) ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4Y1R") ] > 0 &&
             nhit_det[ geomSvc->getDetectorID("H4Y2R") ] > 0   ;
  bool nim1_on = _mode_nim1 == NIM_AND  ?  HYL && HYR  :  HYL || HYR;
  bool nim2_on = _mode_nim2 == NIM_AND  ?  HXT && HXB  :  HXT || HXB;
  _event_header->set_trigger(SQEvent::NIM1, nim1_on);
  _event_header->set_trigger(SQEvent::NIM2, nim2_on);

  //For FPGA trigger, build the internal hit pattern first
  data.clear();
  data.resize(NTRPLANES);
  for(Int_t ihit = 0; ihit < _hit_vector->size(); ++ihit) {
    SQHit *hit=_hit_vector->at(ihit);
    int index = geomSvc->getHodoStation(hit->get_detector_id()) - 1;
    if (0 <= index && index < NTRPLANES) {
      data[index].insert( hit->get_detector_id()*1000 + hit->get_element_id() );
    }
  }
  bool all_planes_have_hits = true;
  for (int i = 0; i < NTRPLANES; ++i) {
    if (data[i].empty())  {
      all_planes_have_hits = false;
      break;
    }
  }

  //do the tree DFS search
  for(int i = 0; i < 2; ++i) {
    path.clear();
    roads_found[i].clear();
    if (all_planes_have_hits) searchMatrix(matrix[i], 0, i);
  }
  
  //FPGA singles trigger
  int nPlusTop      = 0;
  int nPlusBot      = 0;
  int nMinusTop     = 0;
  int nMinusBot     = 0;
  int nHiPxPlusTop  = 0;
  int nHiPxPlusBot  = 0;
  int nHiPxMinusTop = 0;
  int nHiPxMinusBot = 0;
  
  for (std::list<DPTriggerRoad>::iterator iter = roads_found[0].begin();
       iter != roads_found[0].end(); ++iter) {
    if (iter->getTB() > 0) {
      ++nPlusTop;
      if (iter->getPxMin() > 3.) ++nHiPxPlusTop;
    } else {
      ++nPlusBot;
      if (iter->getPxMin() > 3.) ++nHiPxPlusBot;
    }
  }
  
  for (std::list<DPTriggerRoad>::iterator iter = roads_found[1].begin();
       iter != roads_found[1].end(); ++iter) {
    if (iter->getTB() > 0) {
      ++nMinusTop;
      if (iter->getPxMin() > 3.) ++nHiPxMinusTop;
    } else {
      ++nMinusBot;
      if (iter->getPxMin() > 3.) ++nHiPxMinusBot;
    }
  }
  
  bool fpga1_on = (nPlusTop > 0 && nMinusBot > 0) || (nPlusBot  > 0 && nMinusTop > 0);
  bool fpga2_on = (nPlusTop > 0 && nMinusTop > 0) || (nPlusBot  > 0 && nMinusBot > 0);
  bool fpga3_on = (nPlusTop > 0 && nPlusBot  > 0) || (nMinusTop > 0 && nMinusBot > 0);
  bool fpga4_on =  nPlusTop > 0 || nMinusTop > 0  ||  nPlusBot  > 0 || nMinusBot > 0 ;
  bool fpga5_on = nHiPxPlusTop > 0 || nHiPxMinusTop > 0 || nHiPxPlusBot > 0 || nHiPxMinusBot > 0;
  _event_header->set_trigger(SQEvent::MATRIX1, fpga1_on);
  _event_header->set_trigger(SQEvent::MATRIX2, fpga2_on);
  _event_header->set_trigger(SQEvent::MATRIX3, fpga3_on);
  _event_header->set_trigger(SQEvent::MATRIX4, fpga4_on);
  _event_header->set_trigger(SQEvent::MATRIX5, fpga5_on);

  return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::End(PHCompositeNode* topNode) {
  return Fun4AllReturnCodes::EVENT_OK;
}

int DPTriggerAnalyzer::GetNodes(PHCompositeNode* topNode) {
  
  _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!_event_header) {
    LogInfo("!_event_header");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!_hit_vector) {
    LogInfo("!_hit_vector");
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

/// This function converts the lists of roads (i.e. roadset given), into the trees of hodo element nodes.
/**
 * The depth of each branch is always four, which corresponds to the four hodo planes.
 */
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

/// The contents of "buildHitPattern()" have been moved to process_event(),
/// because the similar hit selection was repeated inside and outside this function.

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
    if (data[level].find((*iter)->uniqueID) == data[level].end())
      continue;
    searchMatrix(*iter, level + 1, index);
  }
  path.pop_back();
}

void DPTriggerAnalyzer::deleteMatrix(MatrixNode* node) {
  if (node == NULL) return;

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

////////////////////////////////////////////////////////////////

DPTriggerAnalyzer::MatrixNode::MatrixNode(int uID) : uniqueID(uID)
{
  children.clear();
}

void DPTriggerAnalyzer::MatrixNode::add(MatrixNode* child)
{
  children.push_back(child);
}
