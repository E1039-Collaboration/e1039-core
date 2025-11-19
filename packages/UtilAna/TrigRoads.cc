#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include "TrigRoads.h"
using namespace std;
namespace UtilTrigger {
  
TrigRoads::TrigRoads(const int pol, const int top_bot)
  : m_file_name("")
  , m_pol(pol)
  , m_top_bot(top_bot)
{
  ;
}

TrigRoad* TrigRoads::GetRoad(const int idx)
{
  if (idx < 0 || idx > (int)m_roads.size()) return 0;
  return &m_roads[idx];
}

TrigRoad* TrigRoads::FindRoad(const int road_id)
{
  auto it = m_idx_map.find(road_id);
  if (it != m_idx_map.end()) return &m_roads[it->second];
  return 0;
}
  
int TrigRoads::FindRoadIndex(const int road_id) const
{
  auto it = m_idx_map.find(road_id);
  if (it != m_idx_map.end()) return it->second;
  return -1;
}

const TrigRoad* TrigRoads::GetRoad(const int idx) const
{
  if (idx < 0 || idx > (int)m_roads.size()) return 0;
  return &m_roads[idx];
}

const TrigRoad* TrigRoads::FindRoad(const int road_id) const
{
  auto it = m_idx_map.find(road_id);
  if (it != m_idx_map.end()) return &m_roads[it->second];
  return 0;
}

/// Find all roads included in the given list of road IDs.
/**
 * The return value is a list (vector) of `const TrigRoad*` pointers.
 */
std::vector<const TrigRoad*> TrigRoads::FindRoads(const std::vector<int> list_road_id) const
{
  vector<const TrigRoad*> list_road_ret;
  for (auto it = list_road_id.begin(); it != list_road_id.end(); it++) {
    const TrigRoad* rd = FindRoad(*it);
    if (rd) list_road_ret.push_back(rd);
  }
  return list_road_ret;
}

/// Find all roads included in the given list of road IDs.
/**
 * The return value is a list (vector) of road IDs.
 */
std::vector<int> TrigRoads::FindRoadIDs(const std::vector<int> list_road_id) const
{
  vector<const TrigRoad*> list_road = FindRoads(list_road_id);
  vector<int> list_ret;
  for (auto it = list_road.begin(); it != list_road.end(); it++) list_ret.push_back((*it)->road_id);
  return list_ret;
}
  
int TrigRoads::LoadConfig(const std::string file_name)
{
  //cout << "TrigRoads::LoadConfig(" << file_name << ")\n";
  m_roads.clear();
  m_idx_map.clear();
  m_file_name = file_name;
  ifstream ifs(file_name);
  if (! ifs) return 1;

  int idx = 0;
  string buffer;
  istringstream iss;
  while (getline(ifs, buffer)) {
    if (buffer[0] == '#' || buffer[0] == 'r') continue; // 'r' of 'roadID'
    iss.clear(); // clear any error flags
    iss.str(buffer);

    TrigRoad road;
    if (! (iss >> road.road_id >> road.charge
               >> road.H1X >> road.H2X >> road.H3X >> road.H4X)) continue;
    if (road.road_id * m_top_bot <= 0) continue; // top-bottom mismatch
    if (road.charge  * m_pol     <= 0) continue; // charge mismatch
    m_roads.push_back(road);
    m_idx_map[road.road_id] = idx++;
  }
  ifs.close();
  return 0; // no validation so far
}

std::string TrigRoads::str(const int level) const
{
  ostringstream oss;
  oss << (m_pol > 0 ? '+' : '-') << (m_top_bot > 0 ? 'T' : 'B');
  if (level > 0) oss << "[" << m_roads.size() << "]";
  if (level > 1) {
    for (auto it = m_roads.begin(); it != m_roads.end(); it++) oss << "  " << it->str(level - 2);
  }
  return oss.str();
}

}; // End of "namespace UtilTrigger"
