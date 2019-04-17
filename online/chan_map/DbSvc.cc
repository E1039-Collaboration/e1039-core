#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <TMySQLServer.h>
#include <TSQLStatement.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include "DbSvc.h"
using namespace std;

DbSvc::DbSvc(const SvrId_t svr_id, const UsrId_t usr_id, const std::string my_cnf)
{
  m_svr_id = svr_id;
  m_usr_id = usr_id;
  if (my_cnf.length() > 0) {
    m_my_cnf = my_cnf;
  } else {
    if (usr_id == Guest) m_my_cnf = "/data2/analysis/kenichi/e1039/my.cnf";
    else /* == Prod */   m_my_cnf = "my.cnf"; // not supported yet.
  }
  SelectServer();
  ConnectServer();
}

DbSvc::~DbSvc()
{
  if (m_con) delete m_con;
}

void DbSvc::UseSchema(const char* name, const bool do_create, const bool do_drop)
{
  bool found = false;
  TSQLResult* res = m_con->GetDataBases(name);
  TSQLRow* row = 0;
  while ( (row = res->Next()) != 0 ) {
    string val = row->GetField(0);
    delete row;
    if (val == name) found = true;
  }
  delete res;

  int ret = -1; // non-zero
  if (found) {
    if (do_drop) {
      ret = m_con->DropDataBase(name);
      if (ret == 0) ret = m_con->CreateDataBase(name);
    } else {
      ret = 0; // OK just as found.
    }
  } else {
    if (do_create) ret = m_con->CreateDataBase(name);
  }
  if (ret == 0) ret = m_con->SelectDataBase(name);
  if (ret != 0) {
    cerr << "!!ERROR!!  DbSvc::UseSchema:  Failed to select '" << name << "'.  Abort." << endl;
    exit(1);
  }
}

//void DbSvc::UseSchema(const char* schema, const bool do_create)
//{
//  int ret = m_con->SelectDataBase(schema);
//  if (ret != 0 && do_create) { // Try to create it.
//    ret = m_con->CreateDataBase(schema);
//    if (ret == 0) ret = m_con->SelectDataBase(schema);
//  }
//  if (ret != 0) {
//    cerr << "!!ERROR!!  DbSvc::UseSchema:  Failed to select the schema (" << schema << ").  Abort." << endl;
//    exit(1);
//  }
//}

void DbSvc::DropTable(const char* name)
{
  string query = "drop table if exists ";
  query += name;
  if (! m_con->Exec(query.c_str())) {
    cerr << "!!ERROR!!  DbSvc::DropTable():  Failed on " << name << ".  Abort." << endl;
    exit(1);
  }
}

bool DbSvc::HasTable(const char* name, const bool exit_on_false)
{
  if (m_con->HasTable(name)) return true;
  if (exit_on_false) {
    cerr << "!!ERROR!!  DbSvc:  The table '" << name << "' does not exist.  Abort." << endl;
    exit(1);
  }
  return false;
}

void DbSvc::CreateTable(const std::string name, const std::vector<std::string> list_var, const std::vector<std::string> list_type)
{
  if (HasTable(name)) {
    cerr << "!!ERROR!!  DbSvc::CreateTable():  Table '" << name << "' already exists.  To avoid an unintended creation, please check and drop the table in your code.  Abort." << endl;
    exit(1);
  }
  if (list_var.size() != list_type.size()) {
    cerr << "!!ERROR!!  DbSvc::CreateTable():  The sizes of the var and type lists don't match.  Abort." << endl;
    exit(1);
  }

  ostringstream oss;
  oss << "create table " << name << "(";
  for (unsigned int ii = 0; ii < list_var.size(); ii++) {
    oss << list_var[ii] << " " << list_type[ii] << ", ";
  }
  oss << "primary_id INT not null auto_increment, PRIMARY KEY (primary_id) )";
  if (! m_con->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbSvc::CreateTable():  The creation query failed.  Abort." << endl;
    exit(1);
  }
}

void DbSvc::CreateTable(const std::string name, const int n_var, const char** list_var, const char** list_type)
{
  vector<string> vec_var;
  vector<string> vec_type;
  for (int ii = 0; ii < n_var; ii++) {
    vec_var .push_back(list_var [ii]);
    vec_type.push_back(list_type[ii]);
  }
  CreateTable(name, vec_var, vec_type);
}

/** This function runs Statement(), Process() and StoreResult() with error check.
 *  The return object (TSQLStatement*) must be deleted by user.
 *  Example:
 *   TSQLStatement* stmt = db_svc->Process(query);
 *   while (stmt->NextResultRow()) {
 *     int id    = stmt->GetInt(0);
 *     int event = stmt->GetInt(1);
 *   }
 *   delete stmt;
 */
TSQLStatement* DbSvc::Process(const char* query)
{
  TSQLStatement* stmt = m_con->Statement(query);
  if (! stmt->Process()) {
    cerr << "!!ERROR!!  DbSvc::Process():  Failed to execute a statement:  " << stmt << ".\n"
         << "Abort." << endl;
    exit(1);
  }
  stmt->StoreResult();
  return stmt;
}

void DbSvc::SelectServer()
{
  if      (m_svr_id == DB1 ) m_svr = "e906-db1.fnal.gov";
  else if (m_svr_id == DB2 ) m_svr = "e906-db2.fnal.gov";
  else if (m_svr_id == DB3 ) m_svr = "e906-db3.fnal.gov";
  else if (m_svr_id == DB01) m_svr = "seaquestdb01.fnal.gov:3310";
  else if (m_svr_id == UIUC) m_svr = "seaquel.physics.illinois.edu:3283";
  else {
    cerr << "!!ERROR!!  DbSvc():  Unsupported server ID.  Abort.\n";
    exit(1);
  }
}

void DbSvc::ConnectServer()
{
  ostringstream oss;
  oss << "mysql://" << m_svr << "/?reconnect=1&cnf_file=" << m_my_cnf;

  /// User and password must be given in my.cnf, not here.
  m_con = TMySQLServer::Connect(oss.str().c_str(), 0, 0);
  if (! (m_con && m_con->IsConnected())) {
    cerr << "!!ERROR!!  DbSvc::ConnectServer():  Failed.  Abort." << endl;
    exit(1);
  }
}
