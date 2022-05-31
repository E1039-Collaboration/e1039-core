#include <iostream>
#include <iomanip>
#include <sstream>
#include "DecoParam.h"
using namespace std;

DecoParam::DecoParam()
  : fn_in           ("")
  , dir_param       ("")
  , is_online       (false)
  , sampling        (0)
  , verb            (0)
  , time_wait       (0)
  , runID           (0)
  , spillID         (0)
  , spillID_cntr    (0)
  , spillID_slow    (0)
  , targPos         (0)
  , targPos_slow    (0)
  , event_count     (0)
  , codaID          (0)
  , rocID           (0)
  , eventIDstd      (0)
  , hitID           (0)
  , has_1st_bos     (false)
  , at_bos          (false)
  , turn_id_max     (0)
{
  ;
}

int DecoParam::InitMapper()
{
  //if (dir_param.length() == 0) {
  //  cout << "!!ERROR!!  DecoParam::InitMapper(): dir_param is empty.\n";
  //  return 1;
  //}
  if (runID == 0) {
    cout << "!!ERROR!!  DecoParam::InitMapper(): runID = 0.\n";
    return 1;
  }

  time_t t0 = time(0);
  chan_map_taiwan.SetMapIDbyDB(runID);
  chan_map_taiwan.ReadFromDB();
  chan_map_v1495 .SetMapIDbyDB(runID);
  chan_map_v1495 .ReadFromDB();
  chan_map_scaler.SetMapIDbyDB(runID);
  chan_map_scaler.ReadFromDB();
  time_t t1 = time(0);
  if (t1 - t0 > 3) {
    cout << "DecoParam::InitMapper():  Took too long, " << t1-t0 << " sec.\n"
         << "  Any DNS/DB problem, such as unresponsive DNS server?" << endl;
  }

  return 0;
}
