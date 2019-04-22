/// UploadChanMap.C:  Macro to upload the channel mapping from tsv file to MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int UploadChanMap()
{
  gSystem->Load("libchan_map.so");

  /// Select one of the classes.
  //ChanMapperTaiwan map;
  //ChanMapperV1495  map;
  //ChanMapperScaler map;
  //CalibParamXT map;
  //CalibParamInTimeTaiwan map;
  CalibParamInTimeV1495 map;

  /// Set a map ID.
  const std::string map_id="e906run28740";

  map.SetMapIDbyFile(map_id);
  map.ReadFromFile();
  //map.Print(cout);
  map.WriteToLocalFile("output_for_check.tsv");
  map.WriteToDB();
  map.WriteRangeToDB();

  return 0;
}
