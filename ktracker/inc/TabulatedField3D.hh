// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// Code developed by:
//  S.Larsson and J. Generowicz.
//
//    *************************************
//    *                                   *
//    *    PurgMagTabulatedField3D.hh     *
//    *                                   *
//    *************************************
//
// $Id: PurgMagTabulatedField3D.hh,v 1.3 2006/06/29 16:06:05 gunter Exp $
// GEANT4 tag $Name: geant4-09-01-patch-02 $

#include "globals.hh"
#include "G4MagneticField.hh"
#include "G4ios.hh"
#include "Settings.hh"
#include <fstream>
#include <vector>
#include <cmath>
#include <mysql.h>

using namespace std;

class TabulatedField3D: public G4MagneticField

{
  // Storage space for the table
  vector< vector< vector< double > > > xField;
  vector< vector< vector< double > > > yField;
  vector< vector< vector< double > > > zField;

  // The dimensions of the table
  int nx,ny,nz; 

  // The physical limits of the defined region
  float minx, maxx, miny, maxy, minz, maxz;

  // The physical extent of the defined region
  double dx, dy, dz;
  double fZoffset;
  bool fmag;

  MYSQL* con;

public:
  TabulatedField3D(double, int, int, int, bool, Settings*);
  void  GetFieldValue(const double Point[3], double *Bfield) const;
  Settings* mySettings;
};
