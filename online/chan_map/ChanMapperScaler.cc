#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include "DbSvc.h"
#include "ChanMapperScaler.h"
using namespace std;

ChanMapperScaler::ChanMapperScaler()
{
  m_label = "scaler";
  m_header = "name\troc\tboard\tchan";
}

int ChanMapperScaler::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string name;
    short  roc, board, chan;
    if (! (iss >> name >> roc >> board >> chan)) continue;
    Add(roc, board, chan, name);
    nn++;
  }
  return nn;
}

int ChanMapperScaler::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    string         val = it->second;
    os << val << "\t"
       << std::get<0>(key) << "\t" << std::get<1>(key) << "\t" << std::get<2>(key) << "\n";
    nn++;
  }
  return nn;
}

void ChanMapperScaler::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select roc, board, chan, name from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    short  roc   = stmt->GetInt   (0);
    short  board = stmt->GetInt   (1);
    short  chan  = stmt->GetInt   (2);
    string name  = stmt->GetString(3);
    Add(roc, board, chan, name);
  }
  delete stmt;
}

void ChanMapperScaler::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {      "roc",    "board",     "chan",        "name" };
  const char* list_type[] = { "SMALLINT", "SMALLINT", "SMALLINT", "VARCHAR(64)" };
  db.CreateTable(name_table, 4, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(roc, board, chan, name) values";
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    string         val = it->second;
    oss << " (" << std::get<0>(key) << ", " << std::get<1>(key) << ", " << std::get<2>(key) 
        << ", '" << val << "'),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  ChanMapperScaler::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void ChanMapperScaler::Add(const short roc, const short board, const short chan, const std::string name)
{
  m_map[RocBoardChan_t(roc, board, chan)] = name;
}

bool ChanMapperScaler::Find(const short roc, const short board, const short chan,  std::string& name)
{
  RocBoardChan_t key(roc, board, chan);
  if (m_map.find(key) != m_map.end()) {
    name = m_map[key];
    return true;
  }
  name = "";
  return false;
}  

void ChanMapperScaler::Print(std::ostream& os)
{
  int n_ent = 0;
  for (Map_t::iterator it = m_map.begin(); it != m_map.end(); it++) {
    RocBoardChan_t key = it->first;
    string         val = it->second;
    os << val << "\t"
       << std::get<0>(key) << "\t" << std::get<1>(key) << "\t" << std::get<2>(key) << "\n";
    n_ent++;
  }
  cout << n_ent << endl;
}
