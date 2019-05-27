#include <iomanip>
#include <TGraphErrors.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQHitVector.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <chan_map/CalibParamXT.h>
#include <chan_map/CalibParamInTimeTaiwan.h>
#include "CalibXT.h"
using namespace std;

CalibXT::CalibXT(const std::string& name) : SubsysReco(name), m_cal_xt(0), m_cal_int(0)
, m_db_conf("/seaquest/analysis/kenichi/e1039/my.cnf")
{
  ;
}

CalibXT::~CalibXT()
{
  if (m_cal_xt ) delete m_cal_xt ;
  if (m_cal_int) delete m_cal_int;
}

int CalibXT::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibXT::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;

  if (! m_cal_xt) m_cal_xt = new CalibParamXT();
  m_cal_xt->SetDBConf(m_db_conf);
  m_cal_xt->SetMapIDbyDB(run_header->get_run_id());
  m_cal_xt->ReadFromDB();

  if (! m_cal_int) m_cal_int = new CalibParamInTimeTaiwan();
  m_cal_int->SetDBConf(m_db_conf);
  m_cal_int->SetMapIDbyDB(run_header->get_run_id());
  m_cal_int->ReadFromDB();

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibXT::process_event(PHCompositeNode* topNode)
{
  SQHitVector* hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (! hit_vec) return Fun4AllReturnCodes::ABORTEVENT;

  for (SQHitVector::Iter it = hit_vec->begin(); it != hit_vec->end(); it++) {
    SQHit* hit = *it;
    TGraphErrors* gr_t2x;
    TGraphErrors* gr_t2dx;
    int det = hit->get_detector_id();
    if (m_cal_xt->Find(det, gr_t2x, gr_t2dx)) {
      int ele = hit->get_element_id();
      double center, width;
      if (! m_cal_int->Find(det, ele, center, width)) {
        cerr << "  WARNING:  Cannot find the in-time parameter for det=" << det << " ele=" << ele << " in CalibXT.\n";
        continue;
        //return Fun4AllReturnCodes::ABORTEVENT;
      }
      float t0 = center + width / 2;
      float drift_time = t0 - hit->get_tdc_time();
      hit->set_drift_distance(gr_t2x->Eval(drift_time));
      /// No field for resolution in SQHit now.
      //cout << "check: " << det << " " << ele << " " << t0 << " " << drift_time << " " << hit->get_drift_distance() << endl;
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int CalibXT::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}
