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
#include "UtilOnline.h"
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
  if (++n_evt % 100 == 0) { // Suppress the number of updates
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

/** Function to upload the current run info into DB.
 * Since this function is called quite frequently, the DbSvc object is created only once.
 * Otherwise the number of network connections becomes too many.
 */
void DbUpRun::UploadToDB(SQRun* sq)
{
  const char* table_name = "run";
  static DbSvc* db = 0;
  if (db == 0) {
    db = new DbSvc(DbSvc::DB1);
    db->UseSchema(UtilOnline::GetSchemaMainDaq(), true);
    //db->DropTable(table_name); // Use this when you want to refresh
    if (! db->HasTable(table_name)) {
      DbSvc::VarList list;
      list.Add("run_id"        , "INT", true);
      list.Add("utime_b"       , "INT"); 
      list.Add("utime_e"       , "INT"); 
      list.Add("fpga1_enabled" , "BOOL");
      list.Add("fpga2_enabled" , "BOOL");
      list.Add("fpga3_enabled" , "BOOL");
      list.Add("fpga4_enabled" , "BOOL");
      list.Add("fpga5_enabled" , "BOOL");
      list.Add( "nim1_enabled" , "BOOL");
      list.Add( "nim2_enabled" , "BOOL");
      list.Add( "nim3_enabled" , "BOOL");
      list.Add( "nim4_enabled" , "BOOL");
      list.Add( "nim5_enabled" , "BOOL");
      list.Add("fpga1_prescale", "INT");
      list.Add("fpga2_prescale", "INT");
      list.Add("fpga3_prescale", "INT");
      list.Add("fpga4_prescale", "INT");
      list.Add("fpga5_prescale", "INT");
      list.Add( "nim1_prescale", "INT");
      list.Add( "nim2_prescale", "INT");
      list.Add( "nim3_prescale", "INT");
      list.Add("n_spill"       , "INT"); 
      list.Add("n_evt_all"     , "INT"); 
      list.Add("n_evt_dec"     , "INT");
      db->CreateTable(table_name, list);
    }
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << sq->get_run_id();
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadToDB()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values"
      << " (" << sq->get_run_id() 
      << ", " << sq->get_unix_time_begin()
      << ", " << sq->get_unix_time_end();
  for (int ii=0; ii<5; ii++) oss << ", " << sq->get_fpga_enabled(ii);
  for (int ii=0; ii<5; ii++) oss << ", " << sq->get_nim_enabled(ii);
  for (int ii=0; ii<5; ii++) oss << ", " << sq->get_fpga_prescale(ii);
  for (int ii=0; ii<3; ii++) oss << ", " << sq->get_nim_prescale(ii);
  oss << ", " << sq->get_n_spill()
      << ", " << sq->get_n_evt_all()
      << ", " << sq->get_n_evt_dec()
      << ")";
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadToDB()." << endl;
    return;
  }
}
