/// UploadChanMap.C:  Macro to upload the channel mapping from tsv file to MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int UploadChanMap()
{
  gSystem->Load("libchan_map.so");
  ChanMapperTaiwan map;
  map.SetMapIDbyFile(15000); // todo: use map_id to select the map to be uploaded...
  map.ReadFromFile();

  //map.Print(cout);
  map.WriteToLocalFile("test.tsv");
  map.WriteToDB();
  map.WriteRangeToDB();

  return 0;
}
