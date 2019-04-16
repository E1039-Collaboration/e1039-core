/// MakeChanMap.C:  Macro to create a channel mapping.
R__LOAD_LIBRARY(libchan_map)

int MakeChanMap()
{
  gSystem->Load("libchan_map.so");
  ChanMapperTaiwan map;
  map.SetMapIDbyDB(15000);

  /// roc, board chan, det, ele
  map.Add(10, 100, 1, "D1X", 123);
  map.Add(11, 101, 2, "D2X", 321);

  map.Print(cout);
  map.WriteToLocalFile("test.tsv");
  return 0;
}
