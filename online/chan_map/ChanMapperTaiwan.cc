#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <geom_svc/GeomSvc.h>
#include "DbSvc.h"
#include "ChanMapperTaiwan.h"
using namespace std;

ChanMapperTaiwan::ChanMapperTaiwan()
{
  m_label = "taiwan";
  m_header = "det\tele\troc\tboard\tchan";
  InitNameMap();
}

int ChanMapperTaiwan::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    short  ele, roc, board, chan;
    if (! (iss >> det >> ele >> roc >> board >> chan)) continue;
    Add(roc, board, chan, det, ele);
    nn++;
  }
  return nn;
}

int ChanMapperTaiwan::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->ele << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    nn++;
  }
  return nn;
}

void ChanMapperTaiwan::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select roc, board, chan, det_name, det, ele from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    short  roc      = stmt->GetInt   (0);
    short  board    = stmt->GetInt   (1);
    short  chan     = stmt->GetInt   (2);
    string det_name = stmt->GetString(3);
    short  det      = stmt->GetInt   (4);
    short  ele      = stmt->GetInt   (5);
    Add(roc, board, chan, det_name, det, ele);
  }
  delete stmt;
}

void ChanMapperTaiwan::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {      "roc",    "board",     "chan",    "det_name",      "det",      "ele" };
  const char* list_type[] = { "SMALLINT", "SMALLINT", "SMALLINT", "VARCHAR(32)", "SMALLINT", "SMALLINT" };
  const int   n_var       = 6;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(roc, board, chan, det_name, det, ele) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " (" << it->roc << ", " << it->board << ", " << it->chan
        << ", '" << it->det_name << "', " << it->det << ", " << it->ele << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  ChanMapperTaiwan::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void ChanMapperTaiwan::Add(
  const short roc, const short board, const short chan, 
  const std::string det, const short ele)
{
  GeomSvc* geom = GeomSvc::instance();
  string det_new = det;
  int    ele_new = ele;
  geom->toLocalDetectorName(det_new, ele_new);
  int det_id = geom->getDetectorID(det_new);
  Add(roc, board, chan, det, det_id, ele);

  if (ele_new != ele) {
    cout << "!WARNING!  ChanMapperTaiwan::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
         << "  From det = " << det << ", ele = " << ele << "\n"
         << "  To   det = " << det_new << "(id = " << det_id << "), ele = " << ele_new << "\n"
         << "  The mapping result will be incorrect!!" << endl;
  }

  /// Code to check the conversion
  //int det_id_local = -1;
  //if (m_map_name2id.find(det) != m_map_name2id.end()) det_id_local = m_map_name2id[det];
  //cout << "Debug: " << det << " " << ele << " -> " << det_id << " " << det_new << " " << ele_new << " : "
  //     << det_id - det_id_local << " " << ele - ele_new << "\n";
}

void ChanMapperTaiwan::Add(
  const short roc, const short board, const short chan, 
  const std::string det_name, const short det_id, const short ele)
{
  MapItem item;
  item.roc      = roc;
  item.board    = board;
  item.chan     = chan;
  item.det_name = det_name;
  item.det      = det_id;
  item.ele      = ele;
  m_list.push_back(item);
  m_map[RocBoardChan_t(roc, board, chan)] = DetEle_t(det_id, ele);
}

//bool ChanMapperTaiwan::Find(const short roc, const short board, const short chan,  std::string& det, short& ele)
//{
//  RocBoardChan_t key(roc, board, chan);
//  if (m_map.find(key) != m_map.end()) {
//    DetEle_t* det_ele = &m_map[key];
//    det = det_ele->first;
//    ele = det_ele->second;
//    return true;
//  }
//  det = "";
//  ele = 0;
//  return false;
//}

bool ChanMapperTaiwan::Find(const short roc, const short board, const short chan,  short& det, short& ele)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEle_t* det_ele = &m_map[key];
    det = det_ele->first;
    ele = det_ele->second;
    return true;
  }

//  string det_str;
//  if (! Find(roc, board, chan, det_str, ele)) return false;
//  if (m_map_name2id.find(det_str) != m_map_name2id.end()) {
//    det = m_map_name2id[det_str];
//    return true;
//  }

  det = ele = 0;
  return false;
}

void ChanMapperTaiwan::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    n_ent++;
  }
  cout << "  n = " << n_ent << endl;
}

void ChanMapperTaiwan::InitNameMap()
{
  m_map_name2id["D0U"  ] =   1;
  m_map_name2id["D0Up" ] =   2;
  m_map_name2id["D0X"  ] =   3;
  m_map_name2id["D0Xp" ] =   4;
  m_map_name2id["D0V"  ] =   5;
  m_map_name2id["D0Vp" ] =   6;
  m_map_name2id["D1U"  ] =   1+6;
  m_map_name2id["D1Up" ] =   2+6;
  m_map_name2id["D1X"  ] =   3+6;
  m_map_name2id["D1Xp" ] =   4+6;
  m_map_name2id["D1V"  ] =   5+6;
  m_map_name2id["D1Vp" ] =   6+6;
  m_map_name2id["D2V"  ] =   7+6;
  m_map_name2id["D2Vp" ] =   8+6;
  m_map_name2id["D2Xp" ] =   9+6;
  m_map_name2id["D2X"  ] =  10+6;
  m_map_name2id["D2U"  ] =  11+6;
  m_map_name2id["D2Up" ] =  12+6;
  m_map_name2id["D3pVp"] =  13+6;
  m_map_name2id["D3pV" ] =  14+6;
  m_map_name2id["D3pXp"] =  15+6;
  m_map_name2id["D3pX" ] =  16+6;
  m_map_name2id["D3pUp"] =  17+6;
  m_map_name2id["D3pU" ] =  18+6;
  m_map_name2id["D3mVp"] =  19+6;
  m_map_name2id["D3mV" ] =  20+6;
  m_map_name2id["D3mXp"] =  21+6;
  m_map_name2id["D3mX" ] =  22+6;
  m_map_name2id["D3mUp"] =  23+6;
  m_map_name2id["D3mU" ] =  24+6;
  m_map_name2id["H1B"  ] =  25+6;
  m_map_name2id["H1T"  ] =  26+6;
  m_map_name2id["H1L"  ] =  27+6;
  m_map_name2id["H1R"  ] =  28+6;
  m_map_name2id["H2L"  ] =  29+6;
  m_map_name2id["H2R"  ] =  30+6;
  m_map_name2id["H2B"  ] =  31+6;
  m_map_name2id["H2T"  ] =  32+6;
  m_map_name2id["H3B"  ] =  33+6;
  m_map_name2id["H3T"  ] =  34+6;
  m_map_name2id["H4Y1L"] =  35+6;
  m_map_name2id["H4Y1R"] =  36+6;
  m_map_name2id["H4Y2L"] =  37+6;
  m_map_name2id["H4Y2R"] =  38+6;
  m_map_name2id["H4B"  ] =  39+6;
  m_map_name2id["H4T"  ] =  40+6;
  m_map_name2id["P1Hf" ] =  41+6;
  m_map_name2id["P1Hb" ] =  42+6;
  m_map_name2id["P1Vf" ] =  43+6;
  m_map_name2id["P1Vb" ] =  44+6;
  m_map_name2id["P2Vf" ] =  45+6;
  m_map_name2id["P2Vb" ] =  46+6;
  m_map_name2id["P2Hf" ] =  47+6;
  m_map_name2id["P2Hb" ] =  48+6;

  m_map_name2id["H4Bu"  ] = 39+6;
  m_map_name2id["H4Bd"  ] = 39+6;
  m_map_name2id["H4Tu"  ] = 40+6;
  m_map_name2id["H4Td"  ] = 40+6;
  m_map_name2id["H4Y1Ll"] = 35+6;
  m_map_name2id["H4Y1Lr"] = 35+6;
  m_map_name2id["H4Y1Rl"] = 36+6;
  m_map_name2id["H4Y1Rr"] = 36+6;
  m_map_name2id["H4Y2Ll"] = 37+6;
  m_map_name2id["H4Y2Lr"] = 37+6;
  m_map_name2id["H4Y2Rl"] = 38+6;
  m_map_name2id["H4Y2Rr"] = 38+6;

  for (int ipl = 1; ipl <= 9; ipl++) {
    char tmpName[8][10];
    sprintf(tmpName[0], "P1H%df", ipl);
    sprintf(tmpName[1], "P1H%db", ipl);
    sprintf(tmpName[2], "P1V%df", ipl);
    sprintf(tmpName[3], "P1V%db", ipl);
    sprintf(tmpName[4], "P2V%df", ipl);
    sprintf(tmpName[5], "P2V%db", ipl);
    sprintf(tmpName[6], "P2H%df", ipl);
    sprintf(tmpName[7], "P2H%db", ipl);
    
    m_map_name2id[tmpName[0]] = 41+6;
    m_map_name2id[tmpName[1]] = 42+6;
    m_map_name2id[tmpName[2]] = 43+6;
    m_map_name2id[tmpName[3]] = 44+6;
    m_map_name2id[tmpName[4]] = 45+6;
    m_map_name2id[tmpName[5]] = 46+6;
    m_map_name2id[tmpName[6]] = 47+6;
    m_map_name2id[tmpName[7]] = 48+6;
  }
}
