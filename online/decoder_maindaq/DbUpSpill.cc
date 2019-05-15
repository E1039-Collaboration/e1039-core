/// DbUpSpill.C
#include <iomanip>
#include <TClass.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQSpill.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQStringMap.h>
#include <interface_main/SQScaler.h>
#include <interface_main/SQSlowCont.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <TSQLServer.h>
#include <db_svc/DbSvc.h>
#include "DbUpSpill.h"
using namespace std;

DbUpSpill::DbUpSpill(const std::string& name) : SubsysReco(name)
{
  ;
}

int DbUpSpill::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpSpill::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpSpill::process_event(PHCompositeNode* topNode)
{
  SQSpillMap*     spill_map = findNode::getClass<SQSpillMap >(topNode, "SQSpillMap");
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  if (!spill_map || !event_header) return Fun4AllReturnCodes::ABORTEVENT;

  static int spill_id_pre = -1;
  if (event_header->get_spill_id() != spill_id_pre) {
    spill_id_pre = event_header->get_spill_id();
    SQSpill* spi = spill_map->get(spill_id_pre);
    //PrintSpill(spi);
    UploadToSpillTable(spi);
    UploadToScalerTable(spi, "bos");
    UploadToScalerTable(spi, "eos");
    UploadToSlowContTable(spi);
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int DbUpSpill::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

void DbUpSpill::UploadToSpillTable(SQSpill* spi)
{
  const char* table_name = "spill";
  DbSvc db(DbSvc::DB1);
  db.UseSchema("user_e1039_maindaq", true);
  //db.DropTable(table_name); // Use this when you want to refresh
  if (! db.HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id"      , "INT", true);
    list.Add("spill_id"    , "INT", true); 
    list.Add("target_pos"  , "INT"); 
    list.Add("bos_coda_id" , "INT"); 
    list.Add("bos_vme_time", "INT"); 
    list.Add("eos_coda_id" , "INT"); 
    list.Add("eos_vme_time", "INT"); 
    db.CreateTable(table_name, list);
  }

  ostringstream oss;
  oss << "delete from " << table_name << " where run_id = " << spi->get_run_id() << " and spill_id = " << spi->get_spill_id();
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpSpill::UploadToSpillTable()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values"
      << " (" << spi->get_run_id      () 
      << ", " << spi->get_spill_id    ()
      << ", " << spi->get_target_pos  ()
      << ", " << spi->get_bos_coda_id ()
      << ", " << spi->get_bos_vme_time()
      << ", " << spi->get_eos_coda_id ()
      << ", " << spi->get_eos_vme_time()
      << ")";
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpSpill::UploadToSpillTable()." << endl;
    return;
  }
}

void DbUpSpill::UploadToScalerTable(SQSpill* spi, const std::string boseos)
{
  int   run_id = spi->get_run_id();
  int spill_id = spi->get_spill_id();
  SQStringMap* map_sca;
  if      (boseos == "bos") map_sca = spi->get_bos_scaler_list();
  else if (boseos == "eos") map_sca = spi->get_eos_scaler_list();
  else {
    cerr << "!!ERROR!!  DbUpSpill::UploadToScalerTable(): " << boseos << "?" << endl;
    return;
  }

  ostringstream oss;
  oss << "scaler_" << boseos;
  string table_name = oss.str();

  DbSvc db(DbSvc::DB1);
  db.UseSchema("user_e1039_maindaq", true);
  //db.DropTable(table_name); // Use this when you want to refresh
  if (! db.HasTable(table_name)) {
    DbSvc::VarList list;
    list.Add("run_id"  , "INT", true);
    list.Add("spill_id", "INT", true);
    list.Add("name"    , "VARCHAR(32)", true);
    list.Add("count"   , "INT");
    db.CreateTable(table_name, list);
  }

  oss.str("");
  oss << "delete from " << table_name << " where run_id = " << run_id << " and spill_id = " << spill_id;
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpSpill::UploadToScalerTable()." << endl;
    return;
  }
  oss.str("");
  oss << "insert into " << table_name << " values";
  for (SQStringMap::ConstIter it = map_sca->begin(); it != map_sca->end(); it++) {
    string name = it->first;
    SQScaler* sca = dynamic_cast<SQScaler*>(it->second);
    oss << " (" << run_id << ", " << spill_id << ", '" << name << "', " << sca->get_count() << "),";
  }
  oss.seekp(-1, oss.cur);
  oss << " "; // Erase the last ',' char.
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DbUpSpill::UploadToScalerTable()." << endl;
    return;
  }
}

void DbUpSpill::UploadToSlowContTable(SQSpill* spi)
{
  int   run_id         = spi->get_run_id();
  int spill_id         = spi->get_spill_id();
  SQStringMap* map_slo = spi->get_slow_cont_list();

  typedef map<string, ostringstream> OssMap_t;
  OssMap_t map_oss;
  for (SQStringMap::ConstIter it = map_slo->begin(); it != map_slo->end(); it++) {
    string name = it->first;
    if (name == "U:TODB25") continue; // Known that the value for this name is badly formatted
    SQSlowCont* slo = dynamic_cast<SQSlowCont*>(it->second);
    string type = slo->get_type();
    if (name == "SLOWCONTROL_IS_GOOD") type = "DAQ"; // Known that the type for this name contains extra (invisible) characters.
    map_oss[type] << " (" << run_id << ", " << spill_id << ", '" << name << "', '" << slo->get_time_stamp() << "', '" << slo->get_value() << "'),";
  }

  DbSvc db(DbSvc::DB1);
  db.UseSchema("user_e1039_maindaq", true);

  ostringstream oss;
  for (OssMap_t::iterator it = map_oss.begin(); it != map_oss.end(); it++) {
    string type   = it->first;
    string values = it->second.str();
    if (values.size() == 0) continue;

    string table_name = "slow_cont_";
    table_name += type;
    //db.DropTable(table_name); // Use this when you want to refresh
    if (! db.HasTable(table_name)) {
      DbSvc::VarList list;
      list.Add("run_id"    , "INT", true);
      list.Add("spill_id"  , "INT", true);
      list.Add("name"      , "VARCHAR(64)", true);
      list.Add("time_stamp", "CHAR(14)");
      list.Add("value"     , "TEXT");
      db.CreateTable(table_name, list);
    }
    oss.str("");
    oss << "delete from " << table_name << " where run_id = " << run_id << " and spill_id = " << spill_id;
    if (! db.Con()->Exec(oss.str().c_str())) {
      cerr << "!!ERROR!!  DbUpSpill::UploadToSlowContTable()." << endl;
      continue;
    }
    oss.str("");
    oss << "insert into " << table_name << " values" << values;
    oss.seekp(-1, oss.cur);
    oss << " "; // Erase the last ',' char.
    if (! db.Con()->Exec(oss.str().c_str())) {
      cerr << "!!ERROR!!  DbUpSpill::UploadToSlowContTable()." << endl;
      continue;
    }
  }
}

void DbUpSpill::PrintSpill(SQSpill* spi)
{
  cout << "SQSpill:  "
       << "  " << spi->get_spill_id    ()
       << "  " << spi->get_run_id      ()
       << "  " << spi->get_target_pos  ()
       << "  " << spi->get_bos_coda_id ()
       << "  " << spi->get_bos_vme_time()
       << "  " << spi->get_eos_coda_id ()
       << "  " << spi->get_eos_vme_time()
       << "  \nBOS Scaler:  " << spi->get_bos_scaler_list()->size() << "\n";
  for (SQStringMap::ConstIter it = spi->get_bos_scaler_list()->begin(); it != spi->get_bos_scaler_list()->end(); it++) {
    string name = it->first;
    SQScaler* sca = dynamic_cast<SQScaler*>(it->second);
    cout << "    " << setw(20) << name << " " << setw(10) << sca->get_count() << "\n";
  }
  cout << "  EOS Scaler:  " << spi->get_eos_scaler_list()->size() << "\n";
  for (SQStringMap::ConstIter it = spi->get_eos_scaler_list()->begin(); it != spi->get_eos_scaler_list()->end(); it++) {
    string name = it->first;
    SQScaler* sca = dynamic_cast<SQScaler*>(it->second);
    cout << "  " << setw(20) << name << " " << setw(10) << sca->get_count() << "\n";
  }
  cout << "  Slow Control  " << spi->get_slow_cont_list()->size() << ":\n";
  for (SQStringMap::ConstIter it = spi->get_slow_cont_list()->begin(); it != spi->get_slow_cont_list()->end(); it++) {
    string name = it->first;
    SQSlowCont* slo = dynamic_cast<SQSlowCont*>(it->second);
    cout << "  " << setw(20) << name << " " << slo->get_time_stamp() << " " << setw(20) << slo->get_value() << " " << slo->get_type() << "\n";
  }
}
