/// CheckChanMap.C:  Macro to check one of the channel mappings on MySQL DB.
R__LOAD_LIBRARY(libchan_map)

int CheckChanMap(const int run=25000)
{
  gSystem->Load("libchan_map.so");
  ChanMapperTaiwan map;
  map.SetMapIDbyDB(run);
  map.ReadFromDB();

  //map.Print(cout);
  map.WriteToLocalFile("test.tsv");
  return 0;
}
