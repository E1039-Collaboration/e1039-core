#ifndef _TRIGGER_ROADSET__H_
#define _TRIGGER_ROADSET__H_
#include <string>
#include "TrigRoads.h"

namespace UtilTrigger {
  
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
  //friend std::ostream& operator<<(std::ostream& os, const TrigRoadset& rs);
};

}; // namespace UtilTrigger

#endif // _TRIGGER_ROADSET__H_
