#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <wordexp.h> //to expand environmentals
#include <TMySQLServer.h>
#include <TSQLiteServer.h>
#include <TSQLStatement.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <phool/recoConsts.h>
#include "DbSvc.h"

#define LogInfo(message) std::cout << "DEBUG: " << __FILE__ << "  " << __LINE__ << "  " << __FUNCTION__ << " :::  " << message << std::endl

using namespace std;

DbSvc::DbSvc(const SvrId_t svr_id, const UsrId_t usr_id, const std::string my_cnf)
  : m_svr_id(svr_id)
  , m_usr_id(usr_id)
  , m_my_cnf(my_cnf)
{
  SelectServer();
  SelectUser();
  ConnectServer();
}

DbSvc::DbSvc(const SvrId_t svr_id, const std::string dbfile)
{
  m_svr_id = svr_id;
  if (m_svr_id != LITE) {
    LogInfo("DbSvc::DbSvc(const SvrId_t, const std::string) is for sqlite db only.");
    return;
  }

  std::string m_dbfile = ExpandEnvironmentals(dbfile);
  if (!FileExist(m_dbfile)) {
    LogInfo("SQLITE DB file " << m_dbfile << " does not exist");
    return;
  }

  m_svr = "sqlite://" + m_dbfile;
  m_con = new TSQLiteServer(m_svr.c_str());
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

void DbSvc::CreateTable(const std::string name, const std::vector<std::string> list_var, const std::vector<std::string> list_type, const std::vector<std::string> list_key)
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
  if (list_key.size() == 0) {
    oss << "primary_id INT not null auto_increment, PRIMARY KEY (primary_id)";
  } else {
    oss << "PRIMARY KEY (";
    for (unsigned int ii = 0; ii < list_key.size(); ii++) {
      if (ii != 0) oss << ", ";
      oss << list_key[ii];
    }
    oss << ")";
  }
  oss << ")";
  if (! m_con->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbSvc::CreateTable():  The creation query failed.  Abort." << endl;
    exit(1);
  }
}

void DbSvc::CreateTable(const std::string name, const int n_var, const char** list_var, const char** list_type, const int n_key, const char** list_key)
{
  vector<string> vec_var;
  vector<string> vec_type;
  for (int ii = 0; ii < n_var; ii++) {
    vec_var .push_back(list_var [ii]);
    vec_type.push_back(list_type[ii]);
  }
  vector<string> vec_key;
  for (int ii = 0; ii < n_key; ii++) {
    vec_key.push_back(list_key[ii]);
  }
  CreateTable(name, vec_var, vec_type, vec_key);
}

void DbSvc::CreateTable(const std::string name, const VarList list)
{
  vector<string> vec_var;
  vector<string> vec_type;
  vector<string> vec_key;
  for (unsigned int ii = 0; ii < list.Size(); ii++) {
    string name, type;
    bool is_key;
    list.Get(ii, name, type, is_key);
    vec_var .push_back(name);
    vec_type.push_back(type);
    if (is_key) vec_key.push_back(name);
  }
  CreateTable(name, vec_var, vec_type, vec_key);
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
  if ((! stmt->Process()) && m_svr_id != LITE) {    //sqlite returns false if the query returns no data - this should be accepted
    cerr << "!!ERROR!!  DbSvc::Process():  Failed to execute a statement:  " << stmt << ".\n"
         << "Abort." << endl;
    exit(1);
  }
  stmt->StoreResult();
  return stmt;
}

/**
 * Select a DB server based on the constructor parameter or recoConsts.
 * 'm_svr_id' is not much convenient since a string is expected from recoConsts now.
 * We might better stop using 'm_svr_id' in a future.
 */
void DbSvc::SelectServer()
{
  if (m_svr_id == AutoSvr) {
    recoConsts* rc = recoConsts::instance();
    string svr = rc->get_CharFlag("DB_SERVER");
    if      (svr == "DB1"  ) m_svr_id = DB1  ;
    else if (svr == "DB2"  ) m_svr_id = DB2  ;
    else if (svr == "DB3"  ) m_svr_id = DB3  ;
    else if (svr == "DB4"  ) m_svr_id = DB4  ;
    else if (svr == "LOCAL") m_svr_id = LOCAL;
    else {
      cerr << "!!ERROR!!  DbSvc():  Unsupported DB_SERVER in recoConsts.  Abort.\n";
      exit(1);
    }
  }
  switch (m_svr_id)  {
  case DB1  : m_svr = "e906-db1.fnal.gov";  break;
  case DB2  : m_svr = "e906-db2.fnal.gov";  break;
  case DB3  : m_svr = "e906-db3.fnal.gov";  break;
  case DB4  : m_svr = "e906-db4.fnal.gov";  break;
  case LOCAL: m_svr = "localhost"        ;  break;
  default:
    cerr << "!!ERROR!!  DbSvc():  Unsupported server ID.  Abort.\n";
    exit(1);
  }
}

void DbSvc::SelectUser()
{
  if (m_usr_id == AutoUsr) {
    recoConsts* rc = recoConsts::instance();
    string usr = rc->get_CharFlag("DB_USER");
    if      (usr == "seaguest"  ) m_usr_id = Guest;
    else if (usr == "production") m_usr_id = Prod ;
    else {
      cerr << "!!ERROR!!  DbSvc():  Unsupported DB_USER in recoConsts.  Abort.\n";
      exit(1);
    }
  }

  if (m_my_cnf.length() == 0) {
    if (m_usr_id == Guest) m_my_cnf = "$E1039_RESOURCE/db_conf/guest.cnf";
    else /* == Prod */     m_my_cnf = "$E1039_RESOURCE/db_conf/prod.cnf";
  }
  m_my_cnf = ExpandEnvironmentals(m_my_cnf);
  //LogInfo("Using "<< m_my_cnf);
  if (!FileExist(m_my_cnf)) {
    LogInfo("DB Conf. "<< m_my_cnf << " doesn't exist");
  }
}

void DbSvc::ConnectServer()
{
  ostringstream oss;
  oss << "mysql://" << m_svr << "/?timeout=120&reconnect=1&cnf_file=" << m_my_cnf;
  string url = oss.str();
  
  for (int i_try = 0; i_try < 5; i_try++) { // The connection sometimes fails even with "reconnect=1".  Thus let's try it 5 times.
    m_con = TMySQLServer::Connect(url.c_str(), 0, 0); // User and password must be given in my.cnf, not here.
    if (m_con && m_con->IsConnected()) {
      if (i_try > 0) cout << "DbSvc::ConnectServer():  Succeeded at i_try=" << i_try << "." << endl;
      return; // OK
    }
    sleep(10);
  }

  cerr << "!!ERROR!!  DbSvc::ConnectServer():  Failed.  Abort." << endl;
  exit(1);
}

bool DbSvc::FileExist(const std::string fileName)
{
  std::ifstream infile(fileName.c_str());
  return infile.good();
}

std::string DbSvc::ExpandEnvironmentals( const std::string& input )
{
  // expand any environment variables in the file name
  wordexp_t exp_result;
  if(wordexp(input.c_str(), &exp_result, 0) != 0)
  {
    std::cout << "ExpandEnvironmentals - ERROR - Your string '" << input << "' cannot be understood!" << endl;
    return "";
  }
  const string output( exp_result.we_wordv[0]);
  return output;
}

void DbSvc::VarList::Add(const std::string name, const std::string type, const bool is_key)
{
  m_name  .push_back(name);
  m_type  .push_back(type);
  m_is_key.push_back(is_key);
}

void DbSvc::VarList::Get(const int idx, std::string& name, std::string& type, bool& is_key) const
{
  name   = m_name  [idx];
  type   = m_type  [idx];
  is_key = m_is_key[idx];
}
