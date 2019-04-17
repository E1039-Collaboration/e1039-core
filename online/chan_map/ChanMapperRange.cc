#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "ChanMapperRange.h"
using namespace std;

void ChanMapperRange::Add(const int run_b, const int run_e, const std::string map_id)
{
  RangeItem item;
  item.run_b  = run_b ;
  item.run_e  = run_e ;
  item.map_id = map_id;
  m_list.push_back(item);
}

bool ChanMapperRange::Find(const std::string map_id)
{
  for (RangeList::iterator it = m_list.begin(); it != m_list.end(); it++) {
    if (it->map_id == map_id) return true;
  }
  return false;
}

std::string ChanMapperRange::Find(const int run, const bool exit_on_error)
{
  for (RangeList::iterator it = m_list.begin(); it != m_list.end(); it++) {
    if (it->run_b <= run && run <= it->run_e) return it->map_id;
  }
  if (exit_on_error) {
    cerr << "\n!!ERROR!!  ChanMapperRange::Find():  Cannot find a range for run=" << run << ".  Abort." << endl;
    exit(1);
  }
  return "";
}

void ChanMapperRange::ReadFromFile(const std::string fn_tsv)
{
  cout << "  ChanMapperRange::ReadFromFile(): " << fn_tsv << "...\n";
  ifstream ifs(fn_tsv);
  if (! ifs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << fn_tsv << "'." << endl;
    exit(1);
  } 
  m_list.clear();

  string buffer;
  istringstream iss;
  while ( getline(ifs, buffer) ) {
    if (buffer[0] == '#') continue;
    iss.clear(); // clear any error flags
    iss.str(buffer);

    int  run_b, run_e;
    string map_id;
    if (! (iss >> run_b >> run_e >> map_id)) continue;
    cout << "    " << run_b << " " << run_e << " " << map_id << endl;
    Add(run_b, run_e, map_id);
  }
  ifs.close();
}

void ChanMapperRange::ReadFromDB(const std::string schema)
{
  m_list.clear();
  DbSvc db(DbSvc::DB1);
  db.UseSchema(schema);
  db.HasTable("run_range", true);
  TSQLStatement* stmt = db.Process("select run_b, run_e, map_id from run_range");
  while (stmt->NextResultRow()) {
    int    run_b  = stmt->GetInt   (0);
    int    run_e  = stmt->GetInt   (1);
    string map_id = stmt->GetString(2);
    Add(run_b, run_e, map_id);
  }
  delete stmt;
}

void ChanMapperRange::WriteToDB(const std::string schema)
{
  cout << "ChanMapperRange::WriteToDB()\n";
  cout <<   "  Schema = " << schema << "\n";

  DbSvc db(DbSvc::DB1);
  db.UseSchema(schema, true);
  db.DropTable("run_range");

  const char* list_var [] = { "run_b", "run_e",      "map_id" };
  const char* list_type[] = {   "INT",   "INT", "VARCHAR(64)" };
  db.CreateTable("run_range", 3, list_var, list_type);

  ostringstream oss;
  oss << "insert into run_range (run_b, run_e, map_id) values";
  for (RangeList::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " (" << it->run_b << ", " << it->run_e << ", '" << it->map_id << "'),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "  ERROR in insert.  Abort." << endl;
    exit(1);
  }

  cout <<   "  ...done." << endl;
}
