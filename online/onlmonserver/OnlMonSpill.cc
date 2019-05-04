/// OnlMonSpill.C
#include <iomanip>
//#include <TH1D.h>
//#include <TSocket.h>
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
#include "OnlMonServer.h"
#include "OnlMonSpill.h"
using namespace std;

OnlMonSpill::OnlMonSpill(const std::string& name) : OnlMonClient(name)
{
  ;
}

int OnlMonSpill::Init(PHCompositeNode* topNode)
{
  //Fun4AllHistoManager* hm = new Fun4AllHistoManager(Name());
  //OnlMonServer::instance()->registerHistoManager(hm);
  //hm->registerHisto(h1_tgt);
  //hm->registerHisto(h1_evt_qual);

  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonSpill::InitRun(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonSpill::process_event(PHCompositeNode* topNode)
{
  SQSpillMap*     spill_map = findNode::getClass<SQSpillMap >(topNode, "SQSpillMap");
  SQEvent*     event_header = findNode::getClass<SQEvent    >(topNode, "SQEvent");
  if (!spill_map || !event_header) return Fun4AllReturnCodes::ABORTEVENT;

  static int spill_id_pre = -1;
  if (event_header->get_spill_id() != spill_id_pre) {
    spill_id_pre = event_header->get_spill_id();
    SQSpill* spi = spill_map->get(spill_id_pre);
    //PrintSpill(spi);
    UploadToDB(spi);
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonSpill::End(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int OnlMonSpill::DrawMonitor()
{
  return 0;
}

void OnlMonSpill::UploadToDB(SQSpill* spi)
{
  DbSvc db(DbSvc::DB1);
  db.UseSchema("user_e1039_maindaq", true);
  if (! db.HasTable("spill")) {
    const char* list_var [] = { "run_id", "spill_id", "target_pos", "bos_coda_id", "bos_vme_time", "eos_coda_id", "eos_vme_time" };
    const char* list_type[] = { "INT"   , "INT"     , "INT"       , "INT"        , "INT"         , "INT"        , "INT"          };
    const int   n_var       = 7;
    const char* list_key[] = { "run_id", "spill_id" };
    const int   n_key      = 2;
    db.CreateTable("spill", n_var, list_var, list_type, n_key, list_key);
  }
  
  ostringstream oss;
  oss << "insert into spill (run_id, spill_id, target_pos, bos_coda_id, bos_vme_time, eos_coda_id, eos_vme_time) values ("
      << spi->get_run_id      () << ", "
      << spi->get_spill_id    () << ", "
      << spi->get_target_pos  () << ", "
      << spi->get_bos_coda_id () << ", "
      << spi->get_bos_vme_time() << ", "
      << spi->get_eos_coda_id () << ", "
      << spi->get_eos_vme_time() << ") on duplicate key update target_pos=values(target_pos), bos_coda_id=values(bos_coda_id), bos_vme_time=values(bos_vme_time), eos_coda_id=values(eos_coda_id), eos_vme_time=values(eos_vme_time)";
  if (! db.Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  OnlMonSpill::UploadToDB()." << endl;
    return;
  }
}

void OnlMonSpill::PrintSpill(SQSpill* spi)
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
