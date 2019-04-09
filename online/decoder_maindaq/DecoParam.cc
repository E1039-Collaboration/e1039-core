#include <iostream>
#include <iomanip>
#include <sstream>
#include "DecoParam.h"
using namespace std;

DecoParam::DecoParam() :
  fn_in(""), dir_param(""), sampling(0), verbose(0), time_wait(0), 
  runID(0), spillID(0), spillID_cntr(0), spillID_slow(0),
  targPos(0), targPos_slow(0), 
  codaID(0), rocID(0), hitID(0), 
  n_hit(0), n_thit(0), n_hit_max(0), n_thit_max(0), n_hit_bad(0),
  n_1495_all(0), n_1495_good(0), n_1495_d1ad(0), n_1495_d2ad(0), n_1495_d3ad(0),
  n_phys_evt_all(0), n_phys_evt_dec(0), 
  n_flush_evt_all(0), n_flush_evt_ok(0), at_bos(false), turn_id_max(0)
{
  ;
}

int DecoParam::InitMapper()
{
  if (dir_param.length() == 0) {
    cout << "!!ERROR!!  DecoParam::InitMapper(): dir_param is empty.\n";
    return 1;
  }
  if (runID == 0) {
    cout << "!!ERROR!!  DecoParam::InitMapper(): runID = 0.\n";
    return 1;
  }
  ostringstream oss;
  oss << dir_param << "/run_" << setfill('0') << setw(6) << runID;
  string dir_map = oss.str();
  oss << "/chamber/chamberInfo.tsv";
  map_taiwan.ReadFile(oss.str().c_str());
  oss.str("");
  oss << dir_map << "/hodoscope/hodoInfo.tsv";
  map_taiwan.ReadFile(oss.str().c_str());
  oss.str("");
  oss << dir_map << "/trigger/triggerInfo.tsv";
  map_v1495 .ReadFile(oss.str().c_str());
  oss.str("");
  oss << dir_map << "/scaler/scalerInfo.tsv";
  map_scaler.ReadFile(oss.str().c_str());
  return 0;
}

void DecoParam::PrintStat()
{
  cout << "\nDecoParam::PrintStat():\n"
       << "  Flush events:  all = " << n_flush_evt_all << ", ok = " << n_flush_evt_ok << "\n"
       << "  v1495 events:  all = " << n_1495_all << ", ok = " << n_1495_good
       << ", 0xd1ad = " << n_1495_d1ad << ", 0xd2ad = " << n_1495_d2ad << ", 0xd3ad = " << n_1495_d3ad << "\n"
       << "  Phys. events: all = " << n_phys_evt_all << ", decoded = " << n_phys_evt_dec << "\n"
       << "  TDC   hits: total = " <<  n_hit << ", max/evt = " <<  n_hit_max << ", bad = " << n_hit_bad << "\n"
       << "  v1495 hits: total = " << n_thit << ", max/evt = " << n_thit_max << "\n"
       << "  Real Time: " << (timeEnd - timeStart) << "\n";
}
