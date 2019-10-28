#include <iostream>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include <UtilAna/UtilOnline.h>
#include "DecoStatusDb.h"
using namespace std;

DecoStatusDb::DecoStatusDb() :
  m_name_table ("deco_status")
{
  m_db = new DbSvc(DbSvc::DB1);
  m_db->UseSchema(UtilOnline::GetSchemaMainDaq(), true);

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
    list.Add("run_id" , "INT", true);
    list.Add("deco_status" , "INT");
    list.Add("deco_utime_b", "INT");
    list.Add("deco_utime_e", "INT");
    list.Add("deco_result" , "INT");
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
  oss << "delete from " << m_name_table << " where run_id = " << run;
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
  oss << "update " << m_name_table << " set deco_status = " << FINISHED << ", deco_utime_e = " << utime << ", deco_result = " << result << " where run_id = " << run;
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunFinished()." << endl;
    return;
  }
}
