#include <iostream>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include "DecoStatusDb.h"
using namespace std;

DecoStatusDb::DecoStatusDb() :
  m_name_schema("user_e1039_maindaq_decoder"),
  m_name_table ("deco_status")
{
  m_db = new DbSvc(DbSvc::DB1);
  m_db->UseSchema(m_name_schema, true);

  //m_stat_map["Unknown"    ] = 0;
  //m_stat_map["Started"    ] = 1;
  //m_stat_map["Finished OK"] = 2;
  //m_stat_map["Finished NG"] = 3;
}

void DecoStatusDb::InitTable(const bool refresh)
{

  if (refresh && m_db->HasTable(m_name_table)) m_db->DropTable(m_name_table);
  if (! m_db->HasTable(m_name_table)) {
    DbSvc::VarList list;
    list.Add("run"    , "INT", true);
    list.Add("status" , "INT");
    list.Add("utime_b", "INT");
    list.Add("utime_e", "INT");
    list.Add("result" , "INT");
    m_db->CreateTable(m_name_table, list);
  }

  //const char* table_name = "deco_status_map";
  //if (! m_db->HasTable(table_name)) {
  //  DbSvc::VarList list;
  //  list.Add("status", "INT", true);
  //  list.Add("label" , "VARCHAR(64)");
  //  m_db->CreateTable(table_name, list);
  //}
}

void DecoStatusDb::RunStarted(const int run, int utime)
{
  if (utime == 0) utime = time(0);
  InitTable();

  ostringstream oss;
  oss << "delete from " << m_name_table << " where run = " << run;
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunStarted()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << m_name_table << " values" << " (" << run << ", " << STARTED << ", " << utime << ", 0, 0)";
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunStarted()." << endl;
    return;
  }
}

void DecoStatusDb::RunFinished(const int run, const int result, int utime)
{
  if (utime == 0) utime = time(0);

  ostringstream oss;
  oss << "update " << m_name_table << " set status = " << FINISHED << ", utime_e = " << utime << ", result = " << result << " where run = " << run;
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunFinished()." << endl;
    return;
  }
}
