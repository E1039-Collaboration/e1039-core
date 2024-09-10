#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <TSystem.h>
#include "TrigRoadset.h"
using namespace std;
namespace UtilTrigger {

TrigRoadset::TrigRoadset() 
  : m_dir_conf("")
  , m_roadset(0)
  , m_LBTop(0)
  , m_LBBot(0)
  , m_pos_top(+1, +1)
  , m_pos_bot(+1, -1)
  , m_neg_top(-1, +1)
  , m_neg_bot(-1, -1)
{ 
  ;
}

int TrigRoadset::LoadConfig(const std::string dir)
{
  int ret = 0;
  ret += m_pos_top.LoadConfig(dir+"/rs_LB_pos_top.txt");
  ret += m_pos_bot.LoadConfig(dir+"/rs_LB_pos_bot.txt");
  ret += m_neg_top.LoadConfig(dir+"/rs_LB_neg_top.txt");
  ret += m_neg_bot.LoadConfig(dir+"/rs_LB_neg_bot.txt");
  return ret;
}

int TrigRoadset::LoadConfig(const int roadset_id)
{
  m_roadset = roadset_id;
  ostringstream oss;
  if (m_dir_conf != "") oss << m_dir_conf;
  else oss << gSystem->Getenv("E1039_RESOURCE") << "/trigger/rs";
  oss << "/rs" << roadset_id;
  return LoadConfig(oss.str());
}

int TrigRoadset::LoadConfig(const int firmware_LBTop, const int firmware_LBBot)
{
  ostringstream oss;
  oss << gSystem->Getenv("E1039_RESOURCE") << "/trigger/rs/firmware_ctrl.txt";
  string fn_ctrl = oss.str();
  //cout << "TrigRoadset::LoadConfig(" << firmware_LBTop << ", " << firmware_LBBot << "): " << fn_ctrl << endl;
  ifstream ifs(fn_ctrl);
  if (! ifs) return 1;

  int roadset_id = -1;
  string buffer;
  istringstream iss;
  while (getline(ifs, buffer)) {
    if (buffer[0] == '#') continue;
    iss.clear(); // clear any error flags
    iss.str(buffer);

    int id;
    string str_top, str_bot;
    if (! (iss >> id >> str_top >> str_bot)) continue;
    if (str_top.substr(0, 4) != "0xB0") continue;
    if (str_bot.substr(0, 4) != "0xB1") continue;
    int top = stoi(str_top.substr(2), 0, 16);
    int bot = stoi(str_bot.substr(2), 0, 16);
    if (top == firmware_LBTop && bot == firmware_LBBot) {
      roadset_id = id;
      m_LBTop = top;
      m_LBBot = bot;
      break;
    }
  }
  ifs.close();
  if (roadset_id < 0) return 2;
  return LoadConfig(roadset_id);
}

std::string TrigRoadset::str(const int level) const
{
  ostringstream oss;
  oss << m_roadset;
  if (level > 0) oss << "[" << hex << m_LBTop << "," << m_LBBot << dec << "]";
  if (level > 1) {
    oss << "\n  " << m_pos_top.str(level-2)
        << "\n  " << m_pos_bot.str(level-2)
        << "\n  " << m_neg_top.str(level-2)
        << "\n  " << m_neg_bot.str(level-2);
  }
  return oss.str();
}

}; // End of "namespace UtilTrigger"
