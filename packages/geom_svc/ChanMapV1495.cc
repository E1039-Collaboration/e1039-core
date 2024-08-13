#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "GeomSvc.h"
#include "ChanMapV1495.h"
using namespace std;

ChanMapV1495::ChanMapV1495() :
  ChanMapBase("v1495", "det\tele\tlvl\troc\tboard\tchan")
{
  ;
}

int ChanMapV1495::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    short  ele, lvl, roc, chan;
    int board;
    if (! (iss >> det >> ele >> lvl >> roc >> board >> chan)) continue;
    Add(roc, board, chan, det, ele, lvl);
    nn++;
  }
  return nn;
}

int ChanMapV1495::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->ele << "\t" << it->lvl << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    nn++;
  }
  return nn;
}

void ChanMapV1495::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select roc, board, chan, det_name, det, ele, lvl from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    short  roc      = stmt->GetInt   (0);
    int    board    = stmt->GetInt   (1);
    short  chan     = stmt->GetInt   (2);
    string det_name = stmt->GetString(3);
    short  det      = stmt->GetInt   (4);
    short  ele      = stmt->GetInt   (5);
    short  lvl      = stmt->GetInt   (6);
    Add(roc, board, chan, det_name, det, ele, lvl);
  }
  delete stmt;
}

void ChanMapV1495::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {      "roc", "board",     "chan",    "det_name",      "det",      "ele",      "lvl" };
  const char* list_type[] = { "SMALLINT",   "INT", "SMALLINT", "VARCHAR(32)", "SMALLINT", "SMALLINT", "SMALLINT" };
  const int   n_var       = 7;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(roc, board, chan, det_name, det, ele, lvl) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " (" << it->roc << ", " << it->board << ", " << it->chan
        << ", '" << it->det_name << "', " << it->det << ", " << it->ele << ", " << it->lvl << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  ChanMapV1495::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

/**
 * todo: The "STOP" and "L1PX*" detectors should be handled by GeomSvc properly.
 */
void ChanMapV1495::Add(
  const short roc, const int board, const short chan, 
  const std::string det, const short ele, const short lvl)
{
  GeomSvc* geom = GeomSvc::instance();
  string det_new = det;
  int    ele_new = ele;
  int    det_id;
  if      (det == "STOP"  ) { det_id = 1000; }
  else if (det == "L1PXtp") { det_id = 1001; }
  else if (det == "L1PXtn") { det_id = 1002; }
  else if (det == "L1PXbp") { det_id = 1003; }
  else if (det == "L1PXbn") { det_id = 1004; }
  else {
    geom->toLocalDetectorName(det_new, ele_new);
    det_id = geom->getDetectorID(det_new);
  }
  Add(roc, board, chan, det, det_id, ele, lvl);

  if (ele_new != ele) {
    cout << "!WARNING!  ChanMapV1495::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
         << "  From det = " << det << ", ele = " << ele << "\n"
         << "  To   det = " << det_new << "(id = " << det_id << "), ele = " << ele_new << "\n"
         << "  The mapping result will be incorrect!!" << endl;
  }
}

void ChanMapV1495::Add(
  const short roc, const int board, const short chan, 
  const std::string det_name, const short det_id, const short ele, const short lvl)
{
  MapItem item;
  item.roc      = roc;
  item.board    = board;
  item.chan     = chan;
  item.det_name = det_name;
  item.det      = det_id;
  item.ele      = ele;
  item.lvl      = lvl;
  m_list.push_back(item);
  m_map[RocBoardChan_t(roc, board, chan)] = DetEleLvl_t(det_id, ele, lvl);
}

//bool ChanMapV1495::Find(const short roc, const int board, const short chan,  std::string& det, short& ele, short& lvl)
//{
//  RocBoardChan_t key(roc, board, chan);
//  if (m_map.find(key) != m_map.end()) {
//    DetEleLvl_t val = m_map[key];
//    det = std::get<0>(val);
//    ele = std::get<1>(val);
//    lvl = std::get<2>(val);
//    return true;
//  }
//  det = "";
//  ele = lvl = 0;
//  return false;
//}  

bool ChanMapV1495::Find(const short roc, const int board, const short chan,  short& det, short& ele, short& lvl)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    DetEleLvl_t val = m_map[key];
    det = std::get<0>(val);
    ele = std::get<1>(val);
    lvl = std::get<2>(val);
    return true;
  }

  det = ele = lvl = 0;
  return false;
}

void ChanMapV1495::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t" << it->lvl << "\t"
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    n_ent++;
  }
  cout << n_ent << endl;
}
