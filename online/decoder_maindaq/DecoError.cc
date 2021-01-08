#include <iostream>
#include <iomanip>
#include <sstream>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include <UtilAna/UtilOnline.h>
#include "DecoError.h"
using namespace std;

DecoError::DecoError()
  : m_run_id(0)
  , m_spill_id(0)
  , m_flush_has_error(false)
{
  InitData();
}

void DecoError::SetID(const int run_id, const int spill_id)
{
  m_run_id   = run_id;
  m_spill_id = spill_id;
}

void DecoError::InitData()
{
  m_n_evt_all = 0;
  m_n_evt_ng  = 0;
  for (int roc = 0; roc < N_ROC; roc++) {
    for (int err = 0; err < N_TDC_ERROR; err++) {
      m_n_err_tdc[roc][err].clear();
    }
  }
}

void DecoError::CountFlush()
{
  m_n_evt_all++;
  if (m_flush_has_error) m_n_evt_ng++;
}

void DecoError::AddTdcError(const int event, const int roc, const TdcError_t type)
{
  m_n_err_tdc[roc][type].push_back(event);
  cout << "AddTdcError: " << event << " " << roc << " " << type << "." << endl;
  m_flush_has_error = true;
}

void DecoError::AggregateData()
{
  DbSvc db(DbSvc::DB1);
  db.UseSchema(UtilOnline::GetSchemaMainDaq(), true);
  UpdateDbInfo(&db);
  UpdateDbTdc (&db);
}

void DecoError::PrintData(std::ostream& os)
{
  os << "DecoError::PrintData(): " << m_run_id << " " << m_spill_id << "\n";
  for (int roc = 0; roc < N_ROC; roc++) {
    for (int err = 0; err < N_TDC_ERROR; err++) {
      int n_evt = m_n_err_tdc[roc][err].size();
      if (n_evt == 0) continue;
      os << "  ROC " << setw(2) << roc << ", type " << setw(2) << err << ": n=" << n_evt;
      if (n_evt > 100) n_evt = 100; // Print 100 event IDs at max.
      for (int i_evt = 0; i_evt < n_evt; i_evt++) {
        if (i_evt % 10 == 0) os << "\n    ";
        os << " " << m_n_err_tdc[roc][err].at(i_evt);
      }
      os << "\n";
    }
  }
  os << endl;
}
      
void DecoError::UpdateDbInfo(DbSvc* db)
{
  const char* table_name = "deco_error_info";
  //db->DropTable(table_name);
  if (! db->HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id"   , "INT", true);
    list.Add("spill_id" , "INT");
    list.Add("utime"    , "INT");
    list.Add("n_evt_all", "INT");
    list.Add("n_evt_ng" , "INT");
    db->CreateTable(table_name, list);
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << m_run_id;
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoError::UpdateDbInfo(): delete." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values (" << m_run_id << ", " << m_spill_id << ", " << time(0) << ", " << m_n_evt_all << ", " << m_n_evt_ng << ")";
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoError::UpdateDbInfo(): insert." << endl;
  }
}

void DecoError::UpdateDbTdc(DbSvc* db)
{
  const char* table_name = "deco_error_tdc";
  //db->DropTable(table_name);
  if (! db->HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id"      , "INT", true);
    list.Add("roc_id"      , "INT", true);
    list.Add("error_id"    , "INT", true);
    list.Add("error_count" , "INT");
    list.Add("event_id_min", "INT");
    list.Add("event_id_max", "INT");
    db->CreateTable(table_name, list);
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << m_run_id;
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoError::UpdateDbTdc(): delete." << endl;
    return;
  }

  int n_err = 0;
  oss.str("");
  oss << "insert into " << table_name << " values";
  for (int roc = 0; roc < N_ROC; roc++) {
    for (int err = 0; err < N_TDC_ERROR; err++) {
      if (m_n_err_tdc[roc][err].size() == 0) continue;
      oss << " (" << m_run_id
          << ", " << roc
          << ", " << err
          << ", " << m_n_err_tdc[roc][err].size()
          << ", " << m_n_err_tdc[roc][err].at(0)
          << ", " << m_n_err_tdc[roc][err].back()
          << "),";
      n_err++;
    }
  }
  if (n_err > 0) {
    oss.seekp(-1, oss.cur);
    oss << " "; // Erase the last ',' char.
    if (! db->Con()->Exec(oss.str().c_str())) {
      cerr << "!!ERROR!!  DecoError::UpdateDbTdc()." << endl;
    }
  }
}
