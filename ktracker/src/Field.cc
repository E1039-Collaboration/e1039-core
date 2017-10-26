/* This program sets up the magnetic field variables
Version 05/12/2008
Larry Isenhower modified by Aldo Raeliarijaona
*/
#include "Field.hh"
#include "GlobalConsts.h"

Field::Field(Settings* settings)
{
  mySettings = settings;
  if (settings->asciiFieldMap)
  {
    char *gmcroot, *originalpath, *buf=new char[300];
    long size;
    bool errorFlag = true;

    cout << "Preparing to read magnetic field map file...\n";

    // get path name for directory in which to look for the field map file
    gmcroot = (char *)getenv("GMCROOT");

    size = 1000;
    originalpath = getcwd(buf, size);

    errorFlag = false;

    cout << "Reading field maps..." << endl;
    //                             zOffset, nx, ny, nz, fmag, settings
    Mag1Field= new TabulatedField3D(0.0, 131, 121, 73, true, settings);
    Mag2Field= new TabulatedField3D(-1064.26*cm, 49, 37, 81, false, settings);

    // This exception prevents an error by keeping physiWorld from trying
    // to acces a world volume if no file input was read
    if(errorFlag)
      cout << "No magnetic field input file found! \nPlease check your field map file location and your GMCROOT environment variable." << endl;

   // Move back to original working directory
    chdir(originalpath);
  }
  else // if reading in field map from MySQL
  {
    G4cout << "Preparing to read magnetic field map files...\n";
    G4cout << "Reading field maps...\n";
    Mag1Field= new TabulatedField3D(0.0, 131, 121, 73, true, settings);
    Mag2Field= new TabulatedField3D(-1064.26*cm, 49, 37, 81, false, settings);
  }
  G4cout << "Finished loading magnetic field map files!\n";

  // These should probably be softcoded at some point, but doesn't matter as long as field maps don't get resized.

  zValues[0] = -204.0*cm;  // front of fmag field map
  zValues[1] = 403.74*cm;  // front of kmag field map
  zValues[2] = 712.0*cm;   // end of fmag field map
  zValues[3] = 1572.26*cm; // end of kmag field map
}

Field::~Field()
{
}

void Field::GetFieldValue(const double Point[3], double *Bfield) const
{
  Bfield[0] = 0.;
  Bfield[1] = 0.;
  Bfield[2] = 0.;

  double xTemp = 0;
  double yTemp = 0;
  double zTemp = 0;

  if (Point[2]>zValues[0] && Point[2]<zValues[1])
  {
    Mag1Field->GetFieldValue( Point, Bfield );
  }

  if ((Point[2]>zValues[2])&&(Point[2]<zValues[3]))
  {
    Mag2Field->GetFieldValue( Point, Bfield );
  }

  if ((Point[2]>zValues[1])&&(Point[2]<zValues[2]))
  {
    Mag2Field->GetFieldValue( Point, Bfield );
    xTemp = Bfield[0];
    yTemp = Bfield[1];
    zTemp = Bfield[2];
    Mag1Field->GetFieldValue( Point, Bfield );
    Bfield[0] = Bfield[0] + xTemp;
    Bfield[1] = Bfield[1] + yTemp;
    Bfield[2] = Bfield[2] + zTemp;
  }
}
