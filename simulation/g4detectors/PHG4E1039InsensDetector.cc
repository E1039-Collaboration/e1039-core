#include "PHG4E1039InsensDetector.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4Utils.h>


#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>

#include <Geant4/G4Box.hh>
#include <Geant4/G4Tubs.hh>
#include <Geant4/G4SubtractionSolid.hh>
#include <Geant4/G4Colour.hh>
#include <Geant4/G4LogicalVolume.hh>
#include <Geant4/G4Material.hh>
#include <Geant4/G4PVPlacement.hh>
#include <Geant4/G4SystemOfUnits.hh>
#include <Geant4/G4UserLimits.hh>
#include <Geant4/G4VisAttributes.hh>



#include <sstream>

using namespace std;

//_______________________________________________________________
PHG4E1039InsensDetector::PHG4E1039InsensDetector( PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int lyr):
  PHG4Detector(Node, dnam),
  params(parameters),
  block_physi(nullptr),
  layer(lyr)
{}

//_______________________________________________________________
bool PHG4E1039InsensDetector::IsInBlock(G4VPhysicalVolume * volume) const
{
  if (volume == block_physi)
  {
    return true;
  }
  return false;
}

namespace {
}

//_______________________________________________________________
void PHG4E1039InsensDetector::Construct( G4LogicalVolume* logicWorld )
{
  con = mysql_init(NULL);

  //mysql_real_connect(con,mySettings->sqlServer, mySettings->login, mySettings->password, mySettings->geometrySchema, mySettings->sqlPort, NULL, 0);

  mysql_real_connect(
  		con,
  		"seaquestdb01.fnal.gov",
			"seaguest",
			"qqbar2mu+mu-",
			"geometry_G17_run3",
			3310,
			NULL, 0);

  G4cout << "begin Construct routine..." << endl;

  MYSQL_RES* res;

  MYSQL_ROW row;

  double z, n, a;
  int id;
  G4String name, symbol;

  mysql_query(con, "SELECT eID, eName, symbol, z, n FROM Elements");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);
  int nElement = mysql_num_rows(res);
  elementVec.resize(nElement+1);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)elementVec.size())
      elementVec.resize(id+10);
    name = row[1];
    symbol = row[2];
    z = atof(row[3]);
    n = atof(row[4]);
    a = n * gram;
    elementVec[id] = new G4Element(name, symbol, z, a);
  }

  mysql_free_result(res);

  double density;
  int numEle;
  vector<double> perAbundance;
  perAbundance.resize(11);
  vector<G4String> eleName;
  eleName.resize(11);
  vector<int> eID;
  eID.resize(11);

  //  This query might have to be expanded if materials with more elements are made.
  //  This is inelegant, most of the fields are usually NULL, but I don't know a better way to do it.
  //  Perhaps have all the eIDs and percents in a MySQL VarChar or blob field and parse the string?
  //  It works fine, so not a priority.

  mysql_query(con, "SELECT mID, mName, density, numElements, eID1, eID2, eID3, eID4, eID5, "
              "eID6, eID7, eID8, eID9, eID10, elementPercent1, elementPercent2, "
              "elementPercent3, elementPercent4, elementPercent5, elementPercent6, "
              "elementPercent7, elementPercent8, elementPercent9, elementPercent10 FROM Materials");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);
  int nMaterial = mysql_num_rows(res);
  materialVec.resize(nMaterial+1);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)materialVec.size())
      materialVec.resize(id+10);
    name = row[1];
    density = atof(row[2])*g/(cm*cm*cm);
    numEle = atoi(row[3]);
    for (int i = 1; i <= numEle; i++)
    {
      eID[i] = atoi(row[3+i]);
      perAbundance[i] = atof(row[13+i]);
    }

    materialVec[id] = new G4Material(name, density, numEle);

    for (int j = 1; j <= numEle; j++)
      materialVec[id]->AddElement(elementVec[eID[j]],perAbundance[j]/100.0);
  }

  mysql_free_result(res);

  double xLength, yLength, zLength, radiusMin, radiusMax;
  int shellID, holeID;

  // For any row at least three of the fields will be NULL.  If statements on the shapes are used to define
  // the shapes properly.  More fields might have to be added if one wishes to add new shapes.
  // Compound Solids have to be skipped on the first loop through, then defined in a second loop when all
  // the simple Solids have been defined.

  int nSolid = 3000;
  solidVec.resize(nSolid);

  mysql_query(con, "SELECT sID, sName, xLength, yLength, zLength FROM SolidBoxes");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    xLength = atof(row[2])*cm;
    yLength = atof(row[3])*cm;
    zLength = atof(row[4])*cm;

    solidVec[id] = new G4Box(name, xLength/2.0, yLength/2.0, zLength/2.0);
  }
  mysql_free_result(res);

  mysql_query(con, "SELECT sID, sName, length, radiusMin, radiusMax FROM SolidTubes");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    zLength = atof(row[2])*cm;
    radiusMin = atof(row[3])*cm;
    radiusMax = atof(row[4])*cm;

    solidVec[id] = new G4Tubs(name, radiusMin, radiusMax, zLength/2.0, 0, 360*deg);
  }
  mysql_free_result(res);

  mysql_query(con, "SELECT sID, sName, shellID, holeID, rotX, rotY, rotZ, posX, posY, posZ FROM SubtractionSolids");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)solidVec.size())
      solidVec.resize(id+10);
    name = row[1];

    shellID = atoi(row[2]);
    holeID = atoi(row[3]);

    G4RotationMatrix ra;
    ra.rotateX(atof(row[4]));
    ra.rotateY(atof(row[5]));
    ra.rotateZ(atof(row[6]));
    G4ThreeVector ta;
    ta.setX(atof(row[7]));
    ta.setY(atof(row[8]));
    ta.setZ(atof(row[9]));
    G4Transform3D rata;
    rata = G4Transform3D(ra,ta);

    solidVec[id] = new G4SubtractionSolid(name, solidVec[shellID], solidVec[holeID], rata);
  }
  mysql_free_result(res);

  int sID, mID;
  int sensitiveDetector = 0;

  mysql_query(con, "SELECT lvID, lvName, sID, mID, sensitiveDetector FROM LogicalVolumes");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);
  int nLogicalVolume = mysql_num_rows(res);
  logicalVolumeVec.resize(nLogicalVolume);
  logicalVolumeVec.assign(nLogicalVolume, nullptr);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)logicalVolumeVec.size())
      logicalVolumeVec.resize(id+10);
    name = row[1];
    if(name.find("helium_bag") != std::string::npos) {
      cout << __LINE__ << ": " <<  name << endl;
      continue;
    }
    sID = atoi(row[2]);
    mID = atoi(row[3]);
    sensitiveDetector = atoi(row[4]);
    if(sensitiveDetector == 1) continue;
    if(id == 0)
    	logicalVolumeVec[id] = logicWorld;
    else
    	logicalVolumeVec[id] = new G4LogicalVolume(solidVec[sID], materialVec[mID], name);
  }
  mysql_free_result(res);

  int logicalID, motherID;
  G4ThreeVector pos;

  vector<int> copy;
  copy.resize((int)logicalVolumeVec.size());

  for (int i = 0; i<(int)copy.size(); i++)
    copy[i] = 0;

  mysql_query(con, "SELECT MAX(depth) FROM PhysicalVolumes");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);
  row = mysql_fetch_row(res);
  int depth = atoi(row[0]);

  rotationMatrixVec.resize(0);

  mysql_query(con, "SELECT lvID FROM PhysicalVolumes WHERE depth = 0");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);

  if ((row = mysql_fetch_row(res)))
  {
    logicalID = atoi(row[0]);
    physiWorld = new G4PVPlacement(0, G4ThreeVector(), "World", logicalVolumeVec[logicalID], 0, false, 0);
    copy[logicalID]++;
  }
  mysql_free_result(res);

  char qry[500];

  for (int i = 1; i <= depth; i++)
  {
    sprintf(qry, "SELECT pvID, pvName, lvID, motherID, xRel, yRel, zRel, rotX, rotY, rotZ "
                 "FROM PhysicalVolumes WHERE depth = %i", i);
    mysql_query(con, qry);
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
    res = mysql_store_result(con);

    while ((row = mysql_fetch_row(res)))
    {
      id = atoi(row[0]);
      name = row[1];
      logicalID = atoi(row[2]);
      if(logicalVolumeVec[logicalID] == 0) continue;
      motherID = atoi(row[3]);
      pos(0) = atof(row[4])*cm;
      pos(1) = atof(row[5])*cm;
      pos(2) = atof(row[6])*cm;

      rotationMatrixVec.push_back(new G4RotationMatrix());
      rotationMatrixVec.back()->rotateZ(atof(row[9]));
      rotationMatrixVec.back()->rotateY(atof(row[8]));
      rotationMatrixVec.back()->rotateX(atof(row[7]));

      new G4PVPlacement(rotationMatrixVec.back(), pos, logicalVolumeVec[logicalID], name,
                        logicalVolumeVec[motherID], 0, copy[logicalID]);

      copy[logicalID]++;
    }
    mysql_free_result(res);
  }

  mysql_close(con);
}




























