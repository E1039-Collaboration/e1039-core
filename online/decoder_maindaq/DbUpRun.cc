/// DbUpRun.C
#include <iomanip>
#include <TClass.h>
#include <interface_main/SQRun.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include "DbUpRun.h"
using namespace std;

DbUpRun::DbUpRun(const std::string& name) : SubsysReco(name)
{
  ;
}

int DbUpRun::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpRun::InitRun(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  UploadToDB(run_header);
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpRun::process_event(PHCompositeNode* topNode)
{
  static int n_evt = 0;
  if (++n_evt % 10 == 0) { // Suppress the number of updates
    SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
    if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
    UploadToDB(run_header);
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpRun::End(PHCompositeNode* topNode)
{
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  UploadToDB(run_header);
  return Fun4AllReturnCodes::EVENT_OK;
}

void DbUpRun::UploadToDB(SQRun* sq)
{
  const char* table_name = "run";
  DbSvc db(DbSvc::DB1);
  db.UseSchema("user_e1039_maindaq", true);
  //db.DropTable(table_name); // Use this when you want to refresh
  if (! db.HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id"        , "INT", true);
    list.Add("utime_b"       , "INT"); 
    list.Add("utime_e"       , "INT"); 
    list.Add("n_fee_event"   , "INT"); 
    list.Add("n_fee_prescale", "INT"); 
    list.Add("n_run_desc"    , "INT"); 
    list.Add("n_spill"       , "INT"); 
    list.Add("n_evt_all"     , "INT"); 
    list.Add("n_evt_dec"     , "INT");
    list.Add("n_phys_evt"    , "INT");
    list.Add("n_flush_evt"   , "INT");
    list.Add("n_hit"         , "INT");
    list.Add("n_t_hit"       , "INT");
    list.Add("n_hit_bad"     , "INT");
    list.Add("n_t_hit_bad"   , "INT");
    db.CreateTable(table_name, list);
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << sq->get_run_id();
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadToDB()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values"
      << " (" << sq->get_run_id() 
      << ", " << sq->get_unix_time_begin()
      << ", " << sq->get_unix_time_end()
      << ", " << sq->get_n_fee_event()
      << ", " << sq->get_n_fee_prescale()
      << ", " << sq->get_n_run_desc()
      << ", " << sq->get_n_spill()
      << ", " << sq->get_n_evt_all()
      << ", " << sq->get_n_evt_dec()
      << ", " << sq->get_n_phys_evt()
      << ", " << sq->get_n_flush_evt()
      << ", " << sq->get_n_hit()
      << ", " << sq->get_n_t_hit()
      << ", " << sq->get_n_hit_bad()
      << ", " << sq->get_n_t_hit_bad()
      << ")";
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadToDB()." << endl;
    return;
  }
}
