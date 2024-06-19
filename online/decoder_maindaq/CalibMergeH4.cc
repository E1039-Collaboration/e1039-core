#include <iomanip>
#include <cmath>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <geom_svc/GeomSvc.h>
#include "CalibMergeH4.h"
using namespace std;

CalibMergeH4::CalibMergeH4(const std::string& name)
  : SubsysReco(name)
  , m_and_mode   (false)
  , m_remove_mode(false)
{
  ;
}

CalibMergeH4::~CalibMergeH4()
{
  ;
}

int CalibMergeH4::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibMergeH4::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibMergeH4::process_event(PHCompositeNode* topNode)
{
  SQHitVector*      hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!hit_vec || !trig_hit_vec) return Fun4AllReturnCodes::ABORTEVENT;
  MergeHits(     hit_vec);
  MergeHits(trig_hit_vec);
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibMergeH4::MergeHits(SQHitVector* vec_in)
{
  return m_and_mode ? MergeHitsAnd(vec_in) : MergeHitsOr(vec_in);
}

short CalibMergeH4::FindMergedId(const short id)
{
  if (id == 0) return 0;
  GeomSvc* geom = GeomSvc::instance();
  string name = geom->getDetectorName(id);
  if (name.substr(0, 2) != "H4") return 0;
  string name2 = (name[2] == 'T' || name[2] == 'B') ? name.substr(0, 3) : name.substr(0, 5);
  if (name2 == name) return 0; // The original name doesn't have 'u', 'd', 'l' nor 'r'.
  return geom->getDetectorID(name2);
}

int CalibMergeH4::MergeHitsOr(SQHitVector* vec_in)
{
  for (unsigned int ih = 0; ih < vec_in->size(); ih++) {
    SQHit* hit = vec_in->at(ih);
    short det_new = FindMergedId(hit->get_detector_id());
    if (det_new == 0) continue;
    if (m_remove_mode) {
      hit->set_detector_id(det_new); // Modify ID, which effectively removes the original one.
    } else {
      SQHit* hit_new = hit->Clone();
      hit_new->set_detector_id(det_new);
      vec_in->push_back(hit_new);
    }
  }
  return 0;
}

int CalibMergeH4::MergeHitsAnd(SQHitVector* vec_in)
{
  typedef tuple<short, short, short> MergedGroup_t; // <merged det, element, level>
  typedef map<short, SQHitVector*> MapVec_t; // <det, vector*>
  typedef map<MergedGroup_t, MapVec_t> MapMapVec_t;
  MapMapVec_t map_map_vec;
  // MergedGroup_t represents a group of hits that are merged into one hit.
  // Per merged group we expect two unmerged det IDs (ex. H4Tu & H4Td per H4T),
  // which are used as the key of MapVec_t.
  // Thus MapVec_t usually holds two hit vectors.

  /// Extract H4 hits (and remove them in vec_in)
  for (int ih = vec_in->size() - 1; ih >= 0; ih--) {
    SQHit* hit = vec_in->at(ih);
    short det_org = hit->get_detector_id();
    short det_new = FindMergedId(det_org);
    if (det_new == 0) continue;
    MapVec_t* map_vec = &map_map_vec[MergedGroup_t(det_new, hit->get_element_id(), hit->get_level())];
    if (map_vec->find(det_org) == map_vec->end()) {
      (*map_vec)[det_org] = vec_in->Clone();
      map_vec->at(det_org)->clear();
    }
    map_vec->at(det_org)->push_back(hit);
    if (m_remove_mode) vec_in->erase(ih);
  }

  /// Merge hits per element
  for (MapMapVec_t::iterator it = map_map_vec.begin(); it != map_map_vec.end(); it++) {
    short det_new = std::get<0>(it->first);
    MapVec_t* map_vec = &it->second;
    int n_det = map_vec->size();
    if (n_det == 2) { // Good in the "and" mode.  Merge all hits.
      SQHit* hit_push = 0;
      int    nhit = 0;
      double time = 0;
      for (MapVec_t::iterator it2 = map_vec->begin(); it2 != map_vec->end(); it2++) {
        SQHitVector* vec = it2->second;
        for (SQHitVector::Iter it3 = vec->begin(); it3 != vec->end(); it3++) {
          SQHit* hit = *it3;
          time += hit->get_tdc_time();
          nhit++;
          if (! hit_push) hit_push = hit;
        }
      }
      hit_push->set_tdc_time(time/nhit); // average
      hit_push->set_detector_id(det_new);
      vec_in->push_back(hit_push);
    } else if (n_det > 2) {
      cerr << "CalibMergeH4::MergeHitsAnd():  Unexpectedly found " << map_vec->size() << " detectors per merged detector." << endl;
    }

    for (MapVec_t::iterator it2 = map_vec->begin(); it2 != map_vec->end(); it2++) {
      SQHitVector* vec = it2->second;
      vec->clear();
      delete vec;
    }
  }
  return 0;
}

int CalibMergeH4::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
