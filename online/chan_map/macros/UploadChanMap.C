/// UploadChanMap.C:  Macro to upload the channel mapping from tsv file to MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int UploadChanMap()
{
  gSystem->Load("libchan_map.so");
  ChanMapperTaiwan map;
  map.ReadFromFile(15000);
  //map.ReadFromFile("/data2/analysis/kenichi/e1039/chan_map/chamber/test/chamberInfo_v22.tsv");
  //map.Print(cout);
  //map.WriteToFile("test.tsv");
  map.WriteToDB();

  return 0;
}
