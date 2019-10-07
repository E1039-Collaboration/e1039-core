#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "GeomSvc.h"
#include "ChanMapTaiwan.h"
using namespace std;

ChanMapTaiwan::ChanMapTaiwan() :
  ChanMapBase("taiwan", "det\tele\troc\tboard\tchan")
{
  ;
}

int ChanMapTaiwan::ReadFileCont(LineList& lines)
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

int ChanMapTaiwan::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->ele << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    nn++;
  }
  return nn;
}

void ChanMapTaiwan::ReadDbTable(DbSvc& db)
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

void ChanMapTaiwan::WriteDbTable(DbSvc& db)
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
    cerr << "!!ERROR!!  ChanMapTaiwan::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void ChanMapTaiwan::Add(
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
    cout << "!WARNING!  ChanMapTaiwan::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
         << "  From det = " << det << ", ele = " << ele << "\n"
         << "  To   det = " << det_new << "(id = " << det_id << "), ele = " << ele_new << "\n"
         << "  The mapping result will be incorrect!!" << endl;
  }
}

void ChanMapTaiwan::Add(
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

//bool ChanMapTaiwan::Find(const short roc, const short board, const short chan,  std::string& det, short& ele)
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

bool ChanMapTaiwan::Find(const short roc, const short board, const short chan,  short& det, short& ele)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEle_t* det_ele = &m_map[key];
    det = det_ele->first;
    ele = det_ele->second;
    return true;
  }

  det = ele = 0;
  return false;
}

void ChanMapTaiwan::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    n_ent++;
  }
  cout << "  n = " << n_ent << endl;
}
