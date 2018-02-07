#ifndef Field_H
#define Field_H 1

#include <globals.hh>
#include <G4MagneticField.hh>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include "TabulatedField3D.hh"

class Field : public G4MagneticField
{
  public:
    Field(Settings*);
    ~Field();

    void GetFieldValue(const double Point[3], double *Bfield) const;

  private:
    Settings* mySettings;
    G4double zValues[4];
    G4MagneticField* Mag1Field;
    G4MagneticField* Mag2Field;
};

#endif
