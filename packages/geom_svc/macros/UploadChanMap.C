/// UploadChanMap.C:  Macro to upload the channel mapping from TSV file to MySQL DB.
/**
 * Usage:
 * .L UploadChanMap.C
 * UploadChanMap("taiwan", "2019091301");
 * CheckChanMap ("taiwan", 500);
 */
R__LOAD_LIBRARY(geom_svc)

int UploadChanMap(const std::string type="taiwan", const std::string map_id="2019091301")
{
  ChanMapBase* map;
  if      (type == "taiwan") map = new ChanMapTaiwan();
  else if (type == "v1495" ) map = new ChanMapV1495 ();
  else if (type == "scaler") map = new ChanMapScaler();
  else {
    cerr << "!!ERROR!!  '" << type << "' is not supported." << endl;
    return 1;
  }
  map->SetMapIDbyFile(map_id);
  map->ReadFromFile();
  //map->Print(cout);
  //map->WriteToLocalFile("output_for_check.tsv");
  map->WriteToDB();
  map->WriteRangeToDB();
  return 0;
}

int CheckChanMap(const std::string type="taiwan", const int run=25000)
{
  ChanMapBase* map;
  if      (type == "taiwan") map = new ChanMapTaiwan();
  else if (type == "v1495" ) map = new ChanMapV1495 ();
  else if (type == "scaler") map = new ChanMapScaler();
  else {
    cerr << "!!ERROR!!  '" << type << "' is not supported." << endl;
    return 1;
  }
  map->SetMapIDbyDB(run);
  map->ReadFromDB();
  //map->Print(cout);
  map->WriteToLocalFile("check_chan_map.tsv");
  return 0;
}

/// Test function to make a channel mapping by hand.
int MakeChanMap()
{
  ChanMapTaiwan map;

  /// roc, board chan, det, ele
  map.Add(10, 100, 1, "D1X", 123);
  map.Add(11, 101, 2, "D2X", 321);

  map.Print(cout);
  map.WriteToLocalFile("make_chan_map.tsv");
  return 0;
}
