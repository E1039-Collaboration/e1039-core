#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include "DbSvc.h"
#include "ChanMapperV1495.h"
using namespace std;

ChanMapperV1495::ChanMapperV1495()
{
  m_label = "v1495";
  m_header = "det\tele\tlvl\troc\tboard\tchan";
  InitNameMap();
}

int ChanMapperV1495::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    short  ele, lvl, roc, board, chan;
    if (! (iss >> det >> ele >> lvl >> roc >> board >> chan)) continue;
    Add(roc, board, chan, det, ele, lvl);
    nn++;
  }
  return nn;
}

int ChanMapperV1495::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    DetEleLvl_t    val = it->second;
    os << std::get<0>(val) << "\t" << std::get<1>(val) << "\t" << std::get<2>(val) << "\t"
       << std::get<0>(key) << "\t" << std::get<1>(key) << "\t" << std::get<2>(key) << "\n";
    nn++;
  }
  return nn;
}

void ChanMapperV1495::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select roc, board, chan, det, ele, lvl from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    short  roc   = stmt->GetInt   (0);
    short  board = stmt->GetInt   (1);
    short  chan  = stmt->GetInt   (2);
    string det   = stmt->GetString(3);
    short  ele   = stmt->GetInt   (4);
    short  lvl   = stmt->GetInt   (5);
    Add(roc, board, chan, det, ele, lvl);
  }
  delete stmt;
}

void ChanMapperV1495::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {      "roc",    "board",     "chan",        "det",      "ele",      "lvl" };
  const char* list_type[] = { "SMALLINT", "SMALLINT", "SMALLINT", "VARCHAR(8)", "SMALLINT", "SMALLINT" };
  db.CreateTable(name_table, 6, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(roc, board, chan, det, ele, lvl) values";
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    DetEleLvl_t    val = it->second;
    oss << " ("  << std::get<0>(key) <<  ", " << std::get<1>(key) << ", " << std::get<2>(key) 
        << ", '" << std::get<0>(val) << "', " << std::get<1>(val) << ", " << std::get<2>(val) 
        << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  ChanMapperV1495::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void ChanMapperV1495::Add(const short roc, const short board, const short chan, const std::string det, const short ele, const short lvl)
{
  m_map[RocBoardChan_t(roc, board, chan)] = DetEleLvl_t(det, ele, lvl);
}

bool ChanMapperV1495::Find(const short roc, const short board, const short chan,  std::string& det, short& ele, short& lvl)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEleLvl_t val = m_map[key];
    det = std::get<0>(val);
    ele = std::get<1>(val);
    lvl = std::get<2>(val);
    return true;
  }
  det = "";
  ele = lvl = 0;
  return false;
}  

bool ChanMapperV1495::Find(const short roc, const short board, const short chan,  short& det, short& ele, short& lvl)
{
  string det_str;
  if (! Find(roc, board, chan, det_str, ele, lvl)) return false;
  if (m_map_name2id.find(det_str) != m_map_name2id.end()) {
    det = m_map_name2id[det_str];
    return true;
  }
  det = ele = lvl = 0;
  return false;
}

void ChanMapperV1495::Print(std::ostream& os)
{
  int n_ent = 0;
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    DetEleLvl_t    val = it->second;
    os << std::get<0>(val) << "\t" << std::get<1>(val) << "\t" << std::get<2>(val) << "\t"
       << std::get<0>(key) << "\t" << std::get<1>(key) << "\t" << std::get<2>(key) << "\n";
    n_ent++;
  }
  cout << n_ent << endl;
}

void ChanMapperV1495::InitNameMap()
{
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
}
