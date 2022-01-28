/// UploadCalibParam.C:  Macro to upload the calibration parameter from tsv file to MySQL DB.
/**
 * Usage:
 * .L UploadCalibParam.C
 * UploadCalibParam("xt_curve", "e906run28740");
 * CheckCalibParam ("xt_curve", 25000);
 */
R__LOAD_LIBRARY(geom_svc)

int UploadCalibParam(const std::string type="xt_curve", const std::string map_id="e906run28740")
{
  CalibParamBase* map;
  if      (type == "xt_curve"     ) map = new CalibParamXT();
  else if (type == "intime_taiwan") map = new CalibParamInTimeTaiwan();
  else if (type == "intime_v1495" ) map = new CalibParamInTimeV1495 ();
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

int CheckCalibParam(const std::string type="xt_curve", const int run=25000)
{
  CalibParamBase* map;
  if      (type == "xt_curve"     ) map = new CalibParamXT();
  else if (type == "intime_taiwan") map = new CalibParamInTimeTaiwan();
  else if (type == "intime_v1495" ) map = new CalibParamInTimeV1495 ();
  else {
    cerr << "!!ERROR!!  '" << type << "' is not supported." << endl;
    return 1;
  }
  map->SetMapIDbyDB(run);
  map->ReadFromDB();
  //map->Print(cout);
  map->WriteToLocalFile("check_calib_param.tsv");
  return 0;
}
