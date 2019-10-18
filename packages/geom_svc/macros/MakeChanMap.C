/// MakeChanMap.C:  Macro to create a channel mapping.
R__LOAD_LIBRARY(libgeom_svc)

int MakeChanMap()
{
  gSystem->Load("libgeom_svc.so");
  ChanMapTaiwan map;

  /// roc, board chan, det, ele
  map.Add(10, 100, 1, "D1X", 123);
  map.Add(11, 101, 2, "D2X", 321);

  map.Print(cout);
  map.WriteToLocalFile("make_chan_map.tsv");
  return 0;
}
