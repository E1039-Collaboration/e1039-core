#include <limits>
#include <cmath>
#include "SQRoadList_v1.h"
using namespace std;

ClassImp(SQRoadList_v1);

SQRoadList_v1::SQRoadList_v1()
{
  ;
}

void SQRoadList_v1::identify(ostream& os) const
{
  os << "---SQRoadList_v1--------------------" << endl;
}

int SQRoadList_v1::isValid() const
{
  return 1;
}

void SQRoadList_v1::get_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot) const
{
  pos_top = m_pos_top;
  pos_bot = m_pos_bot;
  neg_top = m_neg_top;
  neg_bot = m_neg_bot;
}

void SQRoadList_v1::set_list(RoadList_t& pos_top, RoadList_t& pos_bot, RoadList_t& neg_top, RoadList_t& neg_bot)
{
  m_pos_top = pos_top;
  m_pos_bot = pos_bot;
  m_neg_top = neg_top;
  m_neg_bot = neg_bot;
}

void SQRoadList_v1::print(std::ostream& os) const
{
  os << "SQRoadList\n"
     << "  pos_top (" << m_pos_top.size() << ")";
  for (auto it = m_pos_top.begin(); it != m_pos_top.end(); it++) os << " " << *it;
  os << "\n";
  os << "  pos_bot (" << m_pos_bot.size() << ")";
  for (auto it = m_pos_bot.begin(); it != m_pos_bot.end(); it++) os << " " << *it;
  os << "\n";
  os << "  neg_top (" << m_neg_top.size() << ")";
  for (auto it = m_neg_top.begin(); it != m_neg_top.end(); it++) os << " " << *it;
  os << "\n";
  os << "  neg_bot (" << m_neg_bot.size() << ")";
  for (auto it = m_neg_bot.begin(); it != m_neg_bot.end(); it++) os << " " << *it;
  os << endl;
}
