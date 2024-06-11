/// DbUpRun.C
#include <iomanip>
#include <TClass.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQParamDeco.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include <UtilAna/UtilOnline.h>
#include "DecoStatusDb.h"
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
  SQRun*       run      = findNode::getClass<SQRun      >(topNode, "SQRun");
  SQParamDeco* par_deco = findNode::getClass<SQParamDeco>(topNode, "SQParamDeco");
  if (!run || !par_deco) return Fun4AllReturnCodes::ABORTEVENT;
  UploadRun(run);
  UploadParam(run->get_run_id(), par_deco);
  UploadV1495(run);
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpRun::process_event(PHCompositeNode* topNode)
{
  static int utime_pre = 0;
  int utime_now = time(0);
  if (utime_now - utime_pre >= 10) { // Suppress the number of updates
    utime_pre = utime_now;
    SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
    if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
    UploadRun(run_header);
    DecoStatusDb deco_stat;
    deco_stat.RunUpdated(run_header->get_run_id());
  }
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpRun::End(PHCompositeNode* topNode)
{
  SQRun*       run      = findNode::getClass<SQRun      >(topNode, "SQRun");
  SQParamDeco* par_deco = findNode::getClass<SQParamDeco>(topNode, "SQParamDeco");
  if (!run || !par_deco) return Fun4AllReturnCodes::ABORTEVENT;
  UploadRun(run);
  UploadParam(run->get_run_id(), par_deco);
  UploadV1495(run);
  return Fun4AllReturnCodes::EVENT_OK;
}

/** Function to upload the current run info into DB.
 * Since this function is called quite frequently, the DbSvc object is created only once.
 * Otherwise the number of network connections becomes too many.
 */
void DbUpRun::UploadRun(SQRun* sq)
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
    cerr << "!!ERROR!!  DbUpRun::UploadRun()." << endl;
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
    cerr << "!!ERROR!!  DbUpRun::UploadRun()." << endl;
    return;
  }
}

/// Function to upload the decoder parameters into DB.
void DbUpRun::UploadParam(const int run, const SQParamDeco* sq)
{
  const char* table_name = "param_deco";
  DbSvc db(DbSvc::DB1);
  db.UseSchema(UtilOnline::GetSchemaMainDaq(), true);
  //db.DropTable(table_name); // Use this when you want to refresh
  if (! db.HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id", "INT", true);
    list.Add("name"  , "VARCHAR(64)", true); 
    list.Add("value" , "VARCHAR(64)"); 
    db.CreateTable(table_name, list);
  }

  if (sq->size() == 0) return;

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << run;
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadParam()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values";
  for (SQParamDeco::ParamConstIter it = sq->begin(); it != sq->end(); it++) {
    oss << " (" << run << ", '" << it->first << "', '" << it->second << "'),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadParam()." << endl;
    return;
  }
}

/// Function to upload the V1495 parameters into DB.
void DbUpRun::UploadV1495(SQRun* sq)
{
  const char* table_name = "v1495";
  static DbSvc* db = 0;
  if (db == 0) {
    db = new DbSvc(DbSvc::DB1);
    db->UseSchema(UtilOnline::GetSchemaMainDaq(), true);
    //db->DropTable(table_name); // Use this when you want to refresh
    if (! db->HasTable(table_name)) {
      DbSvc::VarList list;
      list.Add("run_id"    , "INT", true);
      list.Add("v1495_id_1", "INT"); 
      list.Add("v1495_id_2", "INT"); 
      list.Add("v1495_id_3", "INT"); 
      list.Add("v1495_id_4", "INT"); 
      list.Add("v1495_id_5", "INT"); 
      db->CreateTable(table_name, list);
    }
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << sq->get_run_id();
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadV1495()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values"
      << " (" << sq->get_run_id();
  for (int ii = 0; ii < 5; ii++) oss << ", " << sq->get_v1495_id(ii);
  oss << ")";
  if (! db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpRun::UploadV1495()." << endl;
    return;
  }
}
