#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "ChanMapperScaler.h"
using namespace std;

ChanMapperScaler::ChanMapperScaler() : 
  ChanMapBase("scaler", "name\troc\tboard\tchan")
{
  ;
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
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->name 
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
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
  const int   n_var       = 4;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(roc, board, chan, name) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " (" << it->roc << ", " << it->board << ", " << it->chan
        << ", '" << it->name << "'),";
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
  MapItem item;
  item.roc   = roc;
  item.board = board;
  item.chan  = chan;
  item.name  = name;
  m_list.push_back(item);
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
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->name << "\t" 
       << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
    n_ent++;
  }
  cout << n_ent << endl;
}
