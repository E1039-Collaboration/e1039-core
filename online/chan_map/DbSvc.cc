#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include "DbSvc.h"
using namespace std;

DbSvc::DbSvc(const SvrId_t svr_id)
{
  m_svr_id = svr_id;
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

bool DbSvc::AssureTable(const char* name, const bool exit_on_error)
{
  if (m_con->HasTable(name)) return true;
  if (exit_on_error) {
    cerr << "!!ERROR!!  DbSvc:  The table '" << name << "' does not exist.  Abort." << endl;
    exit(1);
  }
  return false;
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
   //cout << "DB server: " << m_svr << endl;
   string uri = "mysql://";
   uri += m_svr;
   // todo: follow https://root.cern.ch/doc/master/classTMySQLServer.html to use my.cnf.
   // see also /data2/chambers/hvmon_cham/fy2018/src/DBC.cc

   m_con = TMySQLServer::Connect(uri.c_str(), "seaguest", "qqbar2mu+mu-");
   if (! (m_con && m_con->IsConnected())) {
      cerr << "!!ERROR!!  DbSvc::ConnectServer():  Failed.  Abort." << endl;
      exit(1);
   }
}
