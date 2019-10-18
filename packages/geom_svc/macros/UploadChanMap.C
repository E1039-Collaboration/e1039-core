/// UploadChanMap.C:  Macro to upload the channel mapping from tsv file to MySQL DB.
R__LOAD_LIBRARY(libgeom_svc)

int UploadChanMap()
{
  gSystem->Load("libgeom_svc.so");

  /// Select one of the classes.
  ChanMapTaiwan map;
  //ChanMapV1495  map;
  //ChanMapScaler map;
  //CalibParamXT map;
  //CalibParamInTimeTaiwan map;
  //CalibParamInTimeV1495 map;
  //GeomParamPlane map;

  /// Set a map ID.
  const std::string map_id="2019091301";
//  const std::string map_id="e906run28740";
//  const std::string map_id="G9_run5_2";

  map.SetMapIDbyFile(map_id);
  map.ReadFromFile();
  //map.Print(cout);
  map.WriteToLocalFile("output_for_check.tsv");
  map.WriteToDB();
  map.WriteRangeToDB();

  return 0;
}
