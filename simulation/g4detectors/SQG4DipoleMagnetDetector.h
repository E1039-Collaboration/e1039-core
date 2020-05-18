#ifndef SQG4DipoleMagnetDetector_h
#define SQG4DipoleMagnetDetector_h

#include <g4main/PHG4Detector.h>

#include <Geant4/G4RotationMatrix.hh>
#include <Geant4/G4SystemOfUnits.hh>

class G4LogicalVolume;
class PHParameters;
class G4VPhysicalVolume;
class G4Box;
class G4Tubs;
class G4Trd;
class G4SubtractionSolid;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4Material;
class G4Element;
class G4VSolid;

class SQG4DipoleMagnetDetector: public PHG4Detector
{
public:
  //! constructor
  SQG4DipoleMagnetDetector(PHCompositeNode* node, PHParameters* parameters, const std::string& dnam = "BLOCK", const int lyr = 0);

  //! destructor
  virtual ~SQG4DipoleMagnetDetector(void);

  //! construct
  virtual void Construct(G4LogicalVolume* world);

  //!@name volume accessors
  //@{
  bool IsInBlock(G4VPhysicalVolume*) const;
  //@}

  void SuperDetector(const std::string& name) { superdetector = name; }
  const std::string SuperDetector() const { return superdetector; }
  int get_Layer() const { return layer; }

private:
  PHParameters* params;
  G4VPhysicalVolume* block_physi;

  int layer;
  std::string superdetector;
};

#endif
