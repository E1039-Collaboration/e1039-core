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

CalibMergeH4::CalibMergeH4(const std::string& name) : SubsysReco(name), m_and_mode(false), m_remove_mode(false)
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
  //SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  //if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
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
  GeomSvc* geom = GeomSvc::instance();
  typedef tuple<short, short, short> Key_t;
  typedef map<Key_t, SQHitVector*> Map_t;
  Map_t map_vec_h4;

  /// Extract H4 hits (and remove them in vec_in)
  for (int ih = vec_in->size() - 1; ih >= 0; ih--) {
    SQHit* hit = vec_in->at(ih);
    int det = hit->get_detector_id();
    if (det == 0) continue; /// Just skip here.  Must be warned by the channel mapper instead.
    string name = geom->getDetectorName(det);
    if (name.substr(0, 2) == "H4") {
      string name2 = (name[2] == 'T' || name[2] == 'B') ? name.substr(0, 3) : name.substr(0, 5);
      short   det2 = geom->getDetectorID(name2);
      Key_t key(det2, hit->get_element_id(), hit->get_level());
      if (map_vec_h4.find(key) == map_vec_h4.end()) {
        map_vec_h4[key] = vec_in->Clone();
        map_vec_h4[key]->clear();
      }
      map_vec_h4[key]->push_back(hit);
      if (m_remove_mode) vec_in->erase(ih);
    }
  }

  /// Merge hits per element
  for (Map_t::iterator it = map_vec_h4.begin(); it != map_vec_h4.end(); it++) {
    short det = std::get<0>(it->first);
    SQHitVector* vec_h4 = it->second;
    short nh = vec_h4->size();
    if (nh < 1 || (m_and_mode && nh < 2)) continue;
    double time = 0;
    for (int ih = 0; ih < nh; ih++) time += vec_h4->at(ih)->get_tdc_time();
    vec_h4->at(0)->set_tdc_time(time/nh); // average
    vec_h4->at(0)->set_detector_id(det);
    vec_in->push_back(vec_h4->at(0));
    vec_h4->clear();
    delete vec_h4;
  }
}

int CalibMergeH4::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
