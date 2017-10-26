// DetectorConstruction.cc
// need help with this code? jumper1@illinois.edu
//
// This file performs the main fucntion of constructing and configuring
// the physical model of the experiment used in the GMC.  Even though it
// is called DetectorConstruction it constructs all of the geometry, detectors,
// magnets, magnetic fields, concrete barriers, target, etc.  It uses SQL
// databases to construct the shapes, sizes, positions and materials of the
// experiment components, sets up magnetic fields, sets display attributes of
// the physical componets, and assigns which components will be detectors.

#include "DetectorConstruction.hh"
#include "GlobalConsts.h"

DetectorConstruction::DetectorConstruction(Settings* settings)
{
  mySettings = settings;
  G4cout << "detector constructor" << G4endl;
}

DetectorConstruction::~DetectorConstruction()
{
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  con = mysql_init(NULL);

  mysql_real_connect(con,mySettings->sqlServer, mySettings->login, mySettings->password, mySettings->geometrySchema, mySettings->sqlPort, NULL, 0);

  G4cout << "begin Construct routine..." << endl;

  /*    The entire geometry, with the exception of the fields, are built from 5 different Geant Objects.

	Elements (G4Element) are things like carbon and oxygen, but you can define seperate elements
	for seperate isotopes of the same element if you wish.  The database has a liquid deuterium and
	a hydrogen entry, for example.  To define an element you need an atomic number (z), and its
	mass per mole, and a name.

	Materials (G4Material) are built from combinations of one or more elements.  To define a material,
	the number of elements in the material, the pointers to those elements, and the fraction by mass
	for each element.  You also need to give a density and a name for the material.

	Solids (G4VSolid) are the shapes that make up your geometry.  To define one you give the shape
	(i.e. cylinder, box) and the spatial dimensions.  More complex solids can be made by combining
	simpler ones, either by adding them together or by using one solid to define a hole in another
	solid.

	Logical Volumes (G4LogicalVolume) are the combination of a Solid with a Material.  Multiple
	Logical Volumes can use the same Solid.

	Physical Volumes (G4VPhysicalVolume) are what are actually placed within the geometry.  They are
	made from a logical volume, placed within a different logical volume(called its mother volume), and
	given a position within its mother volume.  Multiple Physical Volumes can use the same Logical Volume.
	The World Volume is a special Physical Volume; its Logical Volume contains all the other Physical
	Volumes and it does not have a mother volume.  The World Volume is what this routine returns.

	This routine downloads the Elements info from MySQL and loads it into a vector.  Then it downloads
	the Materials information, builds those using the Elements, and loads it into a different vector.
	It similarly builds up to Physical Volumes.

	At various points certain variables like target length are stored to send to PrimaryGeneratorAction.
	Pointers to the target material and magnet material are also stored so their materials can be changed
	through the UI if desired.
  */

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

  mysql_query(con, "SELECT lvID, lvName, sID, mID FROM LogicalVolumes");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);
  int nLogicalVolume = mysql_num_rows(res);
  logicalVolumeVec.resize(nLogicalVolume);

  while ((row = mysql_fetch_row(res)))
  {
    id = atoi(row[0]);
    if (id >= (int)logicalVolumeVec.size())
      logicalVolumeVec.resize(id+10);
    name = row[1];
    sID = atoi(row[2]);
    mID = atoi(row[3]);
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

  mysql_query(con, "SELECT * FROM ConstantsDerived");
  if (mysql_errno(con) != 0)
    cout << mysql_error(con) << endl;
  res = mysql_store_result(con);

  char constant[30];
  while ((row = mysql_fetch_row(res)))
  {
    sprintf(constant, row[0]);
    if (!strcmp(constant, "targetLength"))
      targetLength = atof(row[1])*cm;
    else if (!strcmp(constant, "targetCenter"))
      targetCenter = atof(row[1])*cm;
    else if (!strcmp(constant, "targetRadius"))
      targetRadius = atof(row[1])*cm;
    else if (!strcmp(constant, "fmagLength"))
      fmagLength = atof(row[1])*cm;
    else if (!strcmp(constant, "fmagCenter"))
      fmagCenter = atof(row[1])*cm;
    else if (!strcmp(constant, "targetMaterialID"))
      targetMat = materialVec[atoi(row[1])];
    else if (!strcmp(constant, "targetVolumeID"))
      targetVolume = logicalVolumeVec[atoi(row[1])];
    else if (!strcmp(constant, "fmagMaterialID"))
      defaultMagnetMat = materialVec[atoi(row[1])];
    else if (!strcmp(constant, "fmagVolumeID"))
      magnetVolume = logicalVolumeVec[atoi(row[1])];
  }
  mysql_free_result(res);

  mysql_close(con);

  //  This manages the physical volumes that are sensitive detectors (particle passing through triggers a hit)

  G4cout << "Initializing sensitive detector manager...\n";
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String sta_SDname = "E906/StationSD";
  staSD = new GenericSD(sta_SDname);
  SDman->AddNewDetector(staSD);
  G4cout << "Finished initializing sensitive detector manager!\n";

  // Execute a function to recursively sort through all Volumes and assign proper display and sensitive detector attributes to each component
  G4cout << "Setting attributes...\n";
  AssignAttributes(physiWorld);
  G4cout << "Finished setting attributes!\n";

  // Magnetic Field
  // For details see Field and TabulatedField3D .cc and .hh

  G4cout << "Setting up physical magnetic field..." << G4endl;

  Field* myField = new Field(mySettings);
  G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(myField);

  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(myField);
  G4MagIntegratorStepper* pStepper = new G4ClassicalRK4(fEquation);
  G4ChordFinder* pChordFinder = new G4ChordFinder(myField,0.01*mm, pStepper);
  fieldMgr->SetChordFinder(pChordFinder);

  G4cout << "******************" << fieldMgr->DoesFieldExist() << "*******************" << G4endl;

  // Prints the material table to screen, for debugging purposes

  G4cout << "here comes the material tables" << endl;
  G4cout << *(G4Element::GetElementTable()) << endl;
  G4cout << *(G4Material::GetMaterialTable()) << endl;
  G4cout << "Geometry complete!!!" << G4endl;

  return physiWorld;
}

int DetectorConstruction::AssignAttributes(G4VPhysicalVolume* mother_phy)
{
  // set all possible display attributes
  G4VisAttributes* targetVisAtt = new G4VisAttributes(G4Colour(0.8,0.8,0.8));
  targetVisAtt->SetForceSolid(true);
  G4VisAttributes* polyVisAtt = new G4VisAttributes(G4Colour(0.7,0.0,0.3,0.3));
  G4VisAttributes* magVisAtt = new G4VisAttributes(G4Colour(0.0,0.6,0.0,0.2));
  G4VisAttributes* coilVisAtt = new G4VisAttributes(G4Colour(0.65,0.83,0.95,0.60));
  G4VisAttributes* concreteVisAtt = new G4VisAttributes(G4Colour(0.45,0.44,0.35,0.8));
  G4VisAttributes* detVisAtt = new G4VisAttributes(G4Colour(0.3,0.5,1.0));
  G4VisAttributes* absWallVisAtt = new G4VisAttributes(G4Colour(0.9,0.88,0.7));
  absWallVisAtt->SetForceSolid(true);

  // check for daughter volumes

  G4LogicalVolume *mother_vol = mother_phy->GetLogicalVolume();
  G4String pName = mother_phy->GetName();
  G4String lName = mother_vol->GetName();
  G4int nDaughters = mother_vol->GetNoDaughters();
  G4Material* mat =  mother_vol->GetMaterial();
  G4String matName = mat->GetName();
  G4int exit = 0;

  bool overDet = lName.contains("osta");
  bool det = ( pName.contains("H1") || pName.contains("H2") || pName.contains("H3") || pName.contains("H4")
            || pName.contains("C1") || pName.contains("C2") || pName.contains("C3")
            || pName.contains("P1") || pName.contains("P2"));

  if (overDet)
    mother_vol->SetSensitiveDetector(staSD);
  else if (det)
    mother_vol->SetSensitiveDetector(staSD);

  //if no daughters exist, determine which attributes to assign
  if(nDaughters == 0)
  {
    // If naming schemes are changed this part needs to be checked for accuracy, as it assigns attributes based on the names of components.
    if (overDet)
      mother_vol->SetVisAttributes(G4VisAttributes::Invisible);
    else if (det)
      mother_vol->SetVisAttributes(detVisAtt);
    else if (matName == "Air" )
      mother_vol->SetVisAttributes(G4VisAttributes::Invisible);
    else if (lName.contains("absorber"))
      mother_vol->SetVisAttributes( absWallVisAtt);
    else if (lName.contains("targ"))
      mother_vol->SetVisAttributes(targetVisAtt);
    else if (lName.contains("poly"))
      mother_vol->SetVisAttributes( polyVisAtt);
    else if (lName.contains("block"))
      mother_vol->SetVisAttributes(concreteVisAtt);
    else if (lName.contains("coil"))
      mother_vol->SetVisAttributes(coilVisAtt);
    else if (lName.contains("concrete"))
      mother_vol->SetVisAttributes(concreteVisAtt);
    else if (lName.contains("body"))
      mother_vol->SetVisAttributes(magVisAtt);
    else
      mother_vol->SetVisAttributes(G4VisAttributes::Invisible);
  }
  else  // if daughters exist, repeat function for each.
  {
    mother_vol->SetVisAttributes(G4VisAttributes::Invisible);
    for(int i=0; i < nDaughters; i++)
      exit += AssignAttributes( mother_vol->GetDaughter(i));
  }

  // 'exit' returns a zero if all componets were assigned attributes or
  // the number of componets missing assigment if some were not assigned
  return exit;
}

// This is called if someone changes the target material through the UI

void DetectorConstruction::SetTargetMaterial(G4String material)
{
  G4NistManager* man = G4NistManager::Instance();
  G4Material* targMat  = man->FindOrBuildMaterial(material);
  targetMat = targMat;
  if (targMat)
    targetVolume->SetMaterial(targetMat);
  else
    cout << material << " not found in material database." << endl;

  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

// This is called if someone changes the magnet material through the UI

void DetectorConstruction::IronToggle(bool iron)
{
  G4NistManager* man = G4NistManager::Instance();
  G4Material* air  = man->FindOrBuildMaterial("Air");

  if (iron == true)
    magnetVolume->SetMaterial(defaultMagnetMat);
  else
    magnetVolume->SetMaterial(air);

  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}

// This is called whenever someone changes the asciiFieldMap bool in the settings
// It reloads the field from the place specified.

void DetectorConstruction::ReloadMagField()
{
  G4cout << "Setting up physical magnetic field..." << G4endl;
  Field* myField = new Field(mySettings);
  G4FieldManager* fieldMgr = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldMgr->SetDetectorField(myField);

  G4Mag_UsualEqRhs* fEquation = new G4Mag_UsualEqRhs(myField);
  G4MagIntegratorStepper* pStepper = new G4ClassicalRK4(fEquation);
  G4ChordFinder* pChordFinder = new G4ChordFinder(myField,1.e-1*mm, pStepper);

  fieldMgr->SetChordFinder(pChordFinder);
  G4cout << "******************" << fieldMgr->DoesFieldExist() << "*******************" << G4endl;

  G4RunManager::GetRunManager()->GeometryHasBeenModified();
}
