#include <string>

#define LogDebug(exp)   std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

using namespace std;

class SolidParam {
public:
  SolidParam(int iid, string iname, double ixLength, double iyLength, double izLength):
  id(iid), name(iname), xLength(ixLength), yLength(iyLength), zLength(izLength)   
{};
  int id;
  string name;
  double xLength;
  double yLength;
  double zLength;
};

void get_phys_param(
    TSQLServer *server,
    const string &pvName,
    int &lvID,
    double *place,
    double *rot
    ){
  char query[2000];
  sprintf(query,
      "SELECT lvID, xRel, yRel, zRel, rotX, rotY, rotZ" 
      " FROM PhysicalVolumes WHERE pvName LIKE '%s_%%'",
      pvName.c_str()
      );
  LogDebug(query);
  TSQLResult *res = server->Query(query);
  int nrow = res->GetRowCount();
  if (nrow!=1) {
    LogDebug("nrow!=1");
  }
  TSQLRow *row = res->Next();
  lvID = atoi((*row)[0]);
  place[0] = atof((*row)[1]);
  place[1] = atof((*row)[2]);
  place[2] = atof((*row)[3]);
  rot[0] = atof((*row)[4]);
  rot[1] = atof((*row)[5]);
  rot[2] = atof((*row)[6]);
  return;
}

void get_logi_param(
    TSQLServer *server,
    const int lvID,
    int &sID
    ){
  char query[2000];
  sprintf(query,
      "SELECT sID" 
      " FROM LogicalVolumes WHERE lvID = %i",
      lvID
      );
  LogDebug(query);
  TSQLResult *res = server->Query(query);
  int nrow = res->GetRowCount();
  if (nrow!=1) {
    LogDebug("nrow!=1");
  }
  TSQLRow *row = res->Next();
  sID = atoi((*row)[0]);
  return;
}

void get_soli_param(
    TSQLServer *server,
    const int sID,
    double *size
    ){
  char query[2000];
  sprintf(query,
      " SELECT xLength, yLength, zLength "
      " FROM SolidBoxes WHERE sID = %i",
      sID
      );
  LogDebug(query);
  TSQLResult *res = server->Query(query);
  int nrow = res->GetRowCount();
  if (nrow!=1) {
    LogDebug("nrow!=1");
  }
  TSQLRow *row = res->Next();
  size[0] = atof((*row)[0]);
  size[1] = atof((*row)[1]);
  size[2] = atof((*row)[2]);
  return;
}

void SetupDriftChamber(
    PHG4Reco *g4Reco,
    std::string _server_name = "seaquestdb01.fnal.gov",
    int _port = 3310,
    std::string _user_name = "seaguest",
    std::string _password = "qqbar2mu+mu-",
    std::string _input_shcema = "geometry_G17_run3"
    ){
  char serverUrl[200];
  sprintf(serverUrl, "mysql://%s:%d", _server_name.c_str(), _port);
  TSQLServer *server = TSQLServer::Connect(serverUrl, _user_name.c_str(), _password.c_str());

  char query[2000];
  sprintf(query, "USE %s", _input_shcema.c_str());

  if(!server->Exec(query))
  {
    std::cout << "MySQLSvc: working schema does not exist! Will exit..." << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  vector<string> chamber_names;
  chamber_names.push_back("C1X");
  chamber_names.push_back("C1V");
  chamber_names.push_back("C1U");
  chamber_names.push_back("C2U");
  chamber_names.push_back("C2X");
  chamber_names.push_back("C2V");
  chamber_names.push_back("C3T");
  chamber_names.push_back("C3B");

  chamber_names.push_back("osta4");
  chamber_names.push_back("osta3");
  chamber_names.push_back("osta2");
  chamber_names.push_back("osta1");

  chamber_names.push_back("H1y");
  chamber_names.push_back("H1x");
  chamber_names.push_back("H2y");
  chamber_names.push_back("H2x");
  chamber_names.push_back("H3x");

  chamber_names.push_back("P1V");
  chamber_names.push_back("P2H");
  chamber_names.push_back("P2V");
  chamber_names.push_back("P1H");

  chamber_names.push_back("H4y1L");
  chamber_names.push_back("H4y1R");
  chamber_names.push_back("H4y2L");
  chamber_names.push_back("H4y2R");
  chamber_names.push_back("H4xT");
  chamber_names.push_back("H4xB");

  for(int i=0; i<chamber_names.size(); ++i){
    string name = chamber_names[i];
    double size[3];
    double place[3];
    double rot[3];
    int lvID;
    int sID;

    LogDebug("");
    get_phys_param(server, name, lvID, place, rot);
    cout
    << "lvID: " << lvID
    << " {" << place[0] << ", " << place[1] << ", " << place[2] << "} "
    << " {" << rot[0] << ", " << rot[1] << ", " << rot[2] << "} "
    << endl;

    LogDebug("");
    get_logi_param(server, lvID, sID);

    LogDebug("");
    get_soli_param(server, sID, size);
    cout
    << "sID: " << sID
    << " {" << size[0] << ", " << size[1] << ", " << size[2] << "} "
    << endl;

    PHG4BlockSubsystem *box = new PHG4BlockSubsystem(name.c_str());
    box->set_double_param("size_x", size[0]);
    box->set_double_param("size_y", size[1]);
    box->set_double_param("size_z", size[2]);
    box->set_double_param("place_x", place[0]);
    box->set_double_param("place_y", place[1]);
    box->set_double_param("place_z", place[2]);
    box->set_string_param("material", "G4_AIR");
    box->SetActive(1);
    g4Reco->registerSubsystem(box);
  }


  return;
}
