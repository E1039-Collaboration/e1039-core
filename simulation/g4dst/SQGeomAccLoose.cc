#include <iomanip>
#include <algorithm>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Hit.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include "SQGeomAccLoose.h"
using namespace std;

SQGeomAccLoose::SQGeomAccLoose(const string& name)
  : SubsysReco(name)
  , m_npl_per_par(4)
  , m_npar_per_evt(2)
{
  ;
}

SQGeomAccLoose::~SQGeomAccLoose()
{
  ;
}

int SQGeomAccLoose::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQGeomAccLoose::InitRun(PHCompositeNode* topNode)
{
  g4hc_h1t  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H1T");
  g4hc_h1b  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H1B");
  g4hc_h2t  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H2T");
  g4hc_h2b  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H2B");
  g4hc_h3t  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H3T");
  g4hc_h3b  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H3B");
  g4hc_h4t  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H4T");
  g4hc_h4b  = findNode::getClass<PHG4HitContainer>(topNode, "G4HIT_H4B");

  if (!g4hc_h1t || !g4hc_h1b || !g4hc_h2t || !g4hc_h2b ||
      !g4hc_h3t || !g4hc_h3b || !g4hc_h4t || !g4hc_h4b   ) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQGeomAccLoose::process_event(PHCompositeNode* topNode)
{
  /// Make lists of particle IDs
  vector<int> vec_id_h1 = ExtractParticleID(g4hc_h1t, g4hc_h1b);
  vector<int> vec_id_h2 = ExtractParticleID(g4hc_h2t, g4hc_h2b);
  vector<int> vec_id_h3 = ExtractParticleID(g4hc_h3t, g4hc_h3b);
  vector<int> vec_id_h4 = ExtractParticleID(g4hc_h4t, g4hc_h4b);

  /// Count the number of hits per particle
  map<int, int> map_nhit; // [particle ID] -> N of hit planes
  CountHitPlanesPerParticle(vec_id_h1, map_nhit);
  CountHitPlanesPerParticle(vec_id_h2, map_nhit);
  CountHitPlanesPerParticle(vec_id_h3, map_nhit);
  CountHitPlanesPerParticle(vec_id_h4, map_nhit);

  /// Count the number of in-acceptance particles
  int n_par_ok = 0;
  for (map<int, int>::iterator it = map_nhit.begin(); it != map_nhit.end(); it++) {
    if (it->second >= m_npl_per_par) n_par_ok++;
  }

  return n_par_ok >= m_npar_per_evt ? Fun4AllReturnCodes::EVENT_OK : Fun4AllReturnCodes::ABORTEVENT;
}

int SQGeomAccLoose::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void SQGeomAccLoose::ExtractParticleID(const PHG4HitContainer* g4hc, vector<int>& vec_par_id)
{
  PHG4HitContainer::ConstRange range = g4hc->getHits();
  for (PHG4HitContainer::ConstIterator it = range.first; it != range.second; it++) {
    PHG4Hit* hit = it->second;
    vec_par_id.push_back(hit->get_trkid());
  }
}

vector<int> SQGeomAccLoose::ExtractParticleID(const PHG4HitContainer* g4hc_t, const PHG4HitContainer* g4hc_b)
{
  vector<int> vec;
  ExtractParticleID(g4hc_t, vec);
  ExtractParticleID(g4hc_b, vec);
  sort(vec.begin(), vec.end());
  vec.erase(unique(vec.begin(), vec.end()), vec.end());
  return vec;
}

void SQGeomAccLoose::CountHitPlanesPerParticle(const vector<int> vec_id, map<int, int>& map_nhit)
{
  for (vector<int>::const_iterator it = vec_id.begin(); it != vec_id.end(); it++) {
    int id = *it;
    if (map_nhit.find(id) == map_nhit.end()) map_nhit[id] = 1;
    else                                     map_nhit[id]++;
  }
}
