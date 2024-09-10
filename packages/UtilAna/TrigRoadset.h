#ifndef _TRIGGER_ROADSET__H_
#define _TRIGGER_ROADSET__H_
#include <string>
#include "TrigRoads.h"

namespace UtilTrigger {

/// Class to handle the trigger roadset.
/**
 * Typical usage:
 * @code
 * #include <UtilAna/TrigRoadset.h>
 * 
 * UtilTrigger::TrigRoadset m_rs;
 * 
 * SQRun* sq_run = findNode::getClass<SQRun>(topNode, "SQRun");
 * if (!sq_run) return Fun4AllReturnCodes::ABORTEVENT;
 * int LBtop = sq_run->get_v1495_id(2);
 * int LBbot = sq_run->get_v1495_id(3);
 * int ret = m_rs.LoadConfig(LBtop, LBbot);
 * if (ret != 0) cout << "LoadConfig() returned " << ret << ".\n";
 * else          cout << "Roadset " << m_rs.str(1) << endl;
 * 
 * int road_pos = trk_pos.getTriggerRoad();
 * int road_neg = trk_neg.getTriggerRoad();
 * bool pos_top = m_rs.PosTop()->FindRoad(road_pos);
 * bool pos_bot = m_rs.PosBot()->FindRoad(road_pos);
 * bool neg_top = m_rs.NegTop()->FindRoad(road_neg);
 * bool neg_bot = m_rs.NegBot()->FindRoad(road_neg);
 * cout << "Roads: " << road_pos << " " << road_neg << " "
 *      << pos_top << pos_bot << neg_top << neg_bot << endl;
 * @endcode
 */
class TrigRoadset {
  std::string m_dir_conf;
  int m_roadset;
  int m_LBTop;
  int m_LBBot;
  TrigRoads m_pos_top;
  TrigRoads m_pos_bot;
  TrigRoads m_neg_top;
  TrigRoads m_neg_bot;
 
 public:  
  TrigRoadset();
  virtual ~TrigRoadset() {;}

  int RoadsetID() const { return m_roadset; }
  int LBTop    () const { return m_LBTop; }
  int LBBot    () const { return m_LBBot; }

  TrigRoads* PosTop() { return &m_pos_top; }
  TrigRoads* PosBot() { return &m_pos_bot; }
  TrigRoads* NegTop() { return &m_neg_top; }
  TrigRoads* NegBot() { return &m_neg_bot; }
  const TrigRoads* PosTop() const { return &m_pos_top; }
  const TrigRoads* PosBot() const { return &m_pos_bot; }
  const TrigRoads* NegTop() const { return &m_neg_top; }
  const TrigRoads* NegBot() const { return &m_neg_bot; }

  int LoadConfig(const std::string dir);
  int LoadConfig(const int roadset_id);
  int LoadConfig(const int firmware_LBTop, const int firmware_LBBot);

  std::string str(const int level=0) const;
};

}; // namespace UtilTrigger

#endif // _TRIGGER_ROADSET__H_
