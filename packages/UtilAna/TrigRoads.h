#ifndef _TRIG_ROADS__H_
#define _TRIG_ROADS__H_
#include <string>
#include <vector>
#include <unordered_map>
#include "TrigRoad.h"

namespace UtilTrigger {
  
class TrigRoads {
  std::string m_file_name;
  int m_pol; // mu+ = +1, mu- = -1
  int m_top_bot; // Top = +1, Bottom = -1
  std::vector<TrigRoad> m_roads;
  std::unordered_map<int, int> m_idx_map;
 
 public:  
  TrigRoads(const int pol, const int top_bot); // const std::string file_name
  virtual ~TrigRoads() {;}

  unsigned int GetNumRoads() const { return m_roads.size(); }
  TrigRoad* GetRoad(const int idx);
  TrigRoad* FindRoad(const int road_id);

  int Charge() const { return m_pol; }
  int TopBot() const { return m_top_bot; }
  int LoadConfig(const std::string file_name);

  std::string str(const int level=0) const;
};

}; // namespace UtilTrigger

#endif // _TRIG_ROADS__H_
