/// CheckChanMap.C:  Macro to check one of the channel mappings on MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int CheckChanMap()
{
  gSystem->Load("libchan_map.so");

  /// Select one of the classes.
  //ChanMapTaiwan map;
  //ChanMapV1495  map;
  //ChanMapScaler map;
  //CalibParamXT map;
  //CalibParamInTimeTaiwan map;
  CalibParamInTimeV1495 map;

  /// Set a run number.
  const int run=25000;

  map.SetMapIDbyDB(run);
  map.ReadFromDB();
  //map.Print(cout);
  map.WriteToLocalFile("check_chan_map.tsv");
  return 0;
}
