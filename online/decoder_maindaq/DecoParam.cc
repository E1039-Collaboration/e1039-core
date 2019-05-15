#include <iostream>
#include <iomanip>
#include <sstream>
#include "DecoParam.h"
using namespace std;

DecoParam::DecoParam() :
  fn_in(""), dir_param(""), sampling(0), verbose(0), time_wait(0), 
  runID(0), spillID(0), spillID_cntr(0), spillID_slow(0),
  targPos(0), targPos_slow(0), codaID(0), rocID(0), hitID(0), 
  at_bos(false), turn_id_max(0)
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

  chan_map_taiwan.SetMapIDbyDB(runID);
  chan_map_taiwan.ReadFromDB();
  chan_map_v1495 .SetMapIDbyDB(runID);
  chan_map_v1495 .ReadFromDB();
  chan_map_scaler.SetMapIDbyDB(runID);
  chan_map_scaler.ReadFromDB();

  return 0;
}

