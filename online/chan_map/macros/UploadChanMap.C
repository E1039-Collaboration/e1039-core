/// UploadChanMap.C:  Macro to upload the channel mapping from tsv file to MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int UploadChanMap(const std::string map_id="e906run28740")
{
  gSystem->Load("libchan_map.so");
  //ChanMapperTaiwan map;
  ChanMapperV1495  map;
  //ChanMapperScaler map;

  map.SetMapIDbyFile(map_id);
  map.ReadFromFile();

  //map.Print(cout);
  map.WriteToLocalFile("test.tsv");
  map.WriteToDB();
  map.WriteRangeToDB();

  return 0;
}
