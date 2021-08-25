#include "SQG4DipoleMagnetDetector.h"

#include <phparameter/PHParameters.h>
#include <g4main/PHG4Utils.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <db_svc/DbSvc.h>

#include <TString.h>
#include <TSQLStatement.h>

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
#include <Geant4/G4NistManager.hh>

#include <iostream>
#include <sstream>
#include <map>

SQG4DipoleMagnetDetector::SQG4DipoleMagnetDetector(PHCompositeNode* node, PHParameters* parameters, const std::string& dnam, const int lyr):
  PHG4Detector(node, dnam),
  params(parameters),
  block_physi(nullptr),
  layer(lyr)
{}

SQG4DipoleMagnetDetector::~SQG4DipoleMagnetDetector() {}

bool SQG4DipoleMagnetDetector::IsInBlock(G4VPhysicalVolume* volume) const
{
  if(volume == block_physi)
  {
    return true;
  }
  return false;
}

void SQG4DipoleMagnetDetector::Construct(G4LogicalVolume* logicWorld)
{
  std::string dbfile = params->get_string_param("geomdb");
  std::string vName  = params->get_string_param("magname");
  std::cout << "SQDipoleMagnet - begin construction of " << vName << " from " << dbfile << std::endl;

  DbSvc* db_svc = new DbSvc(DbSvc::LITE, dbfile.c_str());
  std::unique_ptr<TSQLStatement> stmt;

  // Verbosity(10);
  // overlapcheck = true;

  //Elements and materials are constructed in PHG4Reco::DefineMaterials() - we start from shape contruction

  //Load all shapes
  std::map<int, G4VSolid*> solids;

  /// Read all box shapes from Table SolidBoxes
  stmt.reset(db_svc->Process(     "SELECT sID, sName, xLength, yLength, zLength FROM SolidBoxes WHERE sName like '" + vName + "%'"));
  while(stmt->NextResultRow() && (!stmt->IsNull(0)))
  {
    int sID = stmt->GetInt(0);
    if(solids.find(sID) != solids.end())
    {
      std::cout << "ERROR - SQDipoleMagnet Construction: duplicate solid ID " << sID << std::endl;
      return;
    }

    G4String sName = stmt->GetString(1);
    double xLength = stmt->GetDouble(2)*cm;
    double yLength = stmt->GetDouble(3)*cm;
    double zLength = stmt->GetDouble(4)*cm;
    if(Verbosity() > 1)
    {
      std::cout << "SQDipoleMagnet Construct: create solid box " << sID << " " << sName << " " << xLength << " " << yLength << " " << zLength << std::endl;
    }
    solids[sID] = new G4Box(sName, xLength/2., yLength/2., zLength/2.);
  }

  /// Read all tube shapes from Table SolidTubes
  stmt.reset(db_svc->Process(     "SELECT sID, sName, length, radiusMin, radiusMax FROM SolidTubes WHERE sName like '" + vName + "%'"));
  while(stmt->NextResultRow() && (!stmt->IsNull(0)))
  {
    int sID = stmt->GetInt(0);
    if(solids.find(sID) != solids.end())
    {
      std::cout << "ERROR - SQDipoleMagnet Construction: duplicate solid ID " << sID << std::endl;
      return;
    }

    G4String sName = stmt->GetString(1);
    double length = stmt->GetDouble(2)*cm;
    double radiusMin = stmt->GetDouble(3)*cm;
    double radiusMax = stmt->GetDouble(4)*cm;
    if(Verbosity() > 1)
    {
      std::cout << "SQDipoleMagnet Construct: create solid tube " << sID << " " << sName << " " << length << " " << radiusMax << " " << radiusMin << std::endl;
    }
    solids[sID] = new G4Tubs(sName, radiusMin, radiusMax, length/2., 0., 360.*deg);
  }

  /// Perform subtraction using Table SubtractionSolids
  stmt.reset(db_svc->Process(     "SELECT sID, sName, shellID, holeID, rotX, rotY, rotZ, posX, posY, posZ FROM SubtractionSolids WHERE sName like '" + vName + "%'"));
  while(stmt->NextResultRow() && (!stmt->IsNull(0)))
  {
    int sID = stmt->GetInt(0);
    if(solids.find(sID) != solids.end())
    {
      std::cout << "ERROR - SQDipoleMagnet Construction: duplicate solid ID " << sID << std::endl;
      return;
    }

    G4String sName = stmt->GetString(1);
    int shellID = stmt->GetInt(2);
    int holeID  = stmt->GetInt(3);
    if(Verbosity() > 1)
    {
      std::cout << "SQDipoleMagnet Construct: create solid subtraction " << sID << " " << sName << "  " << shellID << "  " << holeID << std::endl;
    }
    if(solids.find(holeID) == solids.end() || solids.find(shellID) == solids.end())
    {
      std::cout << "ERROR - SQDipoleMagnet Construction: cannot find solid component for " << sName << std::endl;
      return;
    }

    G4RotationMatrix rot;
    rot.rotateX(stmt->GetDouble(4)*rad);
    rot.rotateY(stmt->GetDouble(5)*rad);
    rot.rotateZ(stmt->GetDouble(6)*rad);

    G4ThreeVector pos;
    pos.setX(stmt->GetDouble(7)*cm);
    pos.setY(stmt->GetDouble(8)*cm);
    pos.setZ(stmt->GetDouble(9)*cm);

    G4Transform3D trans(rot, pos);
    solids[sID] = new G4SubtractionSolid(sName, solids[shellID], solids[holeID], trans);
  }

  // Create logical volumes using Table LogicalVolumes
  std::map<int, G4LogicalVolume*> logicals;
  stmt.reset(db_svc->Process(     "SELECT lvID, lvName, sID, mName FROM LogicalVolumes WHERE lvName like '" + vName + "%'"));
  while(stmt->NextResultRow() && (!stmt->IsNull(0)))
  {
    int lvID = stmt->GetInt(0);
    if(logicals.find(lvID) != logicals.end())
    {
      std::cout << "ERROR - SQDipoleMagnet Construction: duplicate logical volume ID " << lvID << std::endl;
      return;
    }

    G4String lvName = stmt->GetString(1);
    int sID = stmt->GetInt(2);
    G4String mName = stmt->GetString(3);
    if(Verbosity() > 1)
    {
      std::cout << "SQDipoleMagnet Construct: create logical volume " << lvID << " " << lvName << " " << sID << " " << mName << std::endl;
    }
    logicals[lvID] = new G4LogicalVolume(solids[sID], G4Material::GetMaterial(mName), lvName);
  }

  // Initialize reference counter for each logical volume
  std::map<int, int> lvRefCount;
  for(auto it = logicals.begin(); it != logicals.end(); ++it)
  {
    lvRefCount[it->first] = 0;
  }

  // Create container volume first by looking for the entry in PhysicalVolumes with motherID=-1
  int nTopPV = 0;
  int topLVID = -1;
  int topPVID = -1;
  std::string topPVName;
  stmt.reset(db_svc->Process(     "SELECT pvID, pvName, lvID FROM PhysicalVolumes WHERE motherID=0 AND depth=1 AND pvName like '" + vName + "%'"));
  while(stmt->NextResultRow())
  {
    ++nTopPV;

    topPVID = stmt->GetInt(0);
    topPVName = stmt->GetString(1);
    topLVID = stmt->GetInt(2);
  }

  if(nTopPV != 1)
  {
    std::cout << "ERROR - SQDipoleMagnet Construction: top volume is ambiguous " << nTopPV << std::endl;
    return;
  }

  if(logicals.find(topLVID) == logicals.end())
  {
    std::cout << "ERROR - SQDipoleMagnet Construction: top logical volume is missing " << std::endl;
    return;
  }

  if(Verbosity() > 1)
  {
    std::cout << "SQDipoleMagnet Construct: create top physical volume " << topPVID << " " << topLVID << " " << topPVName << std::endl;
  }

  // Now create the top volume
  G4RotationMatrix topRot;
  topRot.rotateX(params->get_double_param("rot_x")*rad);
  topRot.rotateY(params->get_double_param("rot_y")*rad);
  topRot.rotateZ(params->get_double_param("rot_z")*rad);

  G4ThreeVector topLoc;
  topLoc.setX(params->get_double_param("place_x")*cm);
  topLoc.setY(params->get_double_param("place_y")*cm);
  topLoc.setZ(params->get_double_param("place_z")*cm);

  block_physi = new G4PVPlacement(G4Transform3D(topRot, topLoc), logicals[topLVID], topPVName.c_str(), logicWorld, false, lvRefCount[topLVID], overlapcheck);
  ++lvRefCount[topLVID];

  // The rest of the physical volumes
  stmt.reset(db_svc->Process(     "SELECT pvName, lvID, motherID, xRel, yRel, zRel, rotX, rotY, rotZ FROM PhysicalVolumes WHERE motherID>0 AND pvName like '" + vName + "%'"));
  while(stmt->NextResultRow() && (!stmt->IsNull(0)))
  {
    //may need to check duplicate pvID?
    G4String pvName = stmt->GetString(0);
    int lvID = stmt->GetInt(1);
    int motherID = stmt->GetInt(2);
    
    G4ThreeVector pos;
    pos.setX(stmt->GetDouble(3)*cm);
    pos.setY(stmt->GetDouble(4)*cm);
    pos.setZ(stmt->GetDouble(5)*cm);

    G4RotationMatrix rot;
    rot.rotateX(stmt->GetDouble(6)*rad);
    rot.rotateY(stmt->GetDouble(7)*rad);
    rot.rotateZ(stmt->GetDouble(8)*rad);

    if(Verbosity() > 1)
    {
      std::cout << "SQDipoleMagnet Construct: create physical volume " << pvName << " " << lvID << " " << motherID << std::endl;
    }

    new G4PVPlacement(G4Transform3D(rot, pos), logicals[lvID], pvName.c_str(), logicals[motherID], false, lvRefCount[lvID], overlapcheck);
    ++lvRefCount[lvID];
  }
}
