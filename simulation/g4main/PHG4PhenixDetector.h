#ifndef PHG4PhenixDetector_h
#define PHG4PhenixDetector_h

#include <Geant4/G4VUserDetectorConstruction.hh>
#include <Geant4/globals.hh>
#include <list>

class PHG4Detector;
class G4Material;
class G4LogicalVolume;
class G4VPhysicalVolume;
class G4FieldManager;

//! this is the main detector construction class, passed to geant to construct the entire phenix detector
class PHG4PhenixDetector: public G4VUserDetectorConstruction
{

  public:

  //! constructor
  PHG4PhenixDetector();

  //! destructor
  virtual ~PHG4PhenixDetector();

  void Verbosity(int verb) {verbosity = verb;}
  
  //! register a detector. This is called in PHG4Reco::Init based on which detectors are found on the tree
  void AddDetector(PHG4Detector* detector, int zero_field = 0)
  { detectors_.push_back(detector); zeroFieldFlags.push_back(zero_field); }

  //! this is called by geant to actually construct all detectors
  virtual G4VPhysicalVolume* Construct( void );

  //! this is used to associate the local field manager to the no-field-zone logical volume
  virtual void ConstructSDandField();

  G4double GetWorldSizeX() const
  {return WorldSizeX;}

  G4double GetWorldSizeY() const
  {return WorldSizeY;}
  G4double GetWorldSizeZ() const
  {return WorldSizeZ;}

  void SetWorldSizeX(const G4double sx) {WorldSizeX = sx;}
  void SetWorldSizeY(const G4double sy) {WorldSizeY = sy;}
  void SetWorldSizeZ(const G4double sz) {WorldSizeZ = sz;}

  void SetWorldShape(const std::string &s) {worldshape = s;}
  void SetWorldMaterial(const std::string &s) {worldmaterial = s;}
  G4VPhysicalVolume* GetPhysicalVolume(void) {return physiWorld;}

  void SetZeroFieldStartZ(const G4double z) { ZeroFieldStartZ = z; }
  void SetZeroFieldManager(G4FieldManager* man) { zeroFieldManager = man; }

  protected:

  private:
  int verbosity;
  
  //! list of detectors to be constructed
  typedef std::list<PHG4Detector*> DetectorList;
  DetectorList detectors_;
  std::list<int> zeroFieldFlags;

  G4Material* defaultMaterial;

  G4LogicalVolume* logicWorld; //pointer to the logical World
  G4VPhysicalVolume* physiWorld; //pointer to the physical World
  G4double WorldSizeX;
  G4double WorldSizeY;
  G4double WorldSizeZ;
  std::string worldshape;
  std::string worldmaterial;

  G4double ZeroFieldStartZ;
  G4LogicalVolume*   lZeroFieldSubWorld;
  G4VPhysicalVolume* pZeroFieldSubWorld;
  G4FieldManager*    zeroFieldManager;
};

#endif
