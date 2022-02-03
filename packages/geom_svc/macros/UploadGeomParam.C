/// UploadGeomParam.C:  Macro to upload the geometry parameter from tsv file to MySQL DB.
/**
 * Usage:
 * .L UploadGeomParam.C
 * UploadGeomParam("plane", "G9_run5_2");
 * CheckGeomParam(25000);
 */
R__LOAD_LIBRARY(geom_svc)

int UploadGeomParam(const std::string type="plane", const std::string map_id="G9_run5_2")
{
  GeomParamPlane map;
  map.SetMapIDbyFile(map_id);
  map.ReadFromFile();
  //map.Print(cout);
  //map.WriteToLocalFile("output_for_check.tsv");
  map.WriteToDB();
  map.WriteRangeToDB();
  return 0;
}

int CheckGeomParam(const int run=25000)
{
  GeomParamPlane map;
  map.SetMapIDbyDB(run);
  map.ReadFromDB();
  map.WriteToLocalFile("check_geom_param.tsv");
  return 0;
}
