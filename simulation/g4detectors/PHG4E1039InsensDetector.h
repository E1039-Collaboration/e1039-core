#ifndef PHG4E1039InsensDetector_h
#define PHG4E1039InsensDetector_h

#include <g4main/PHG4Detector.h>

#include <Geant4/G4RotationMatrix.hh>
#include <Geant4/G4SystemOfUnits.hh>

#include <mysql/mysql.h>

#include <vector>

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

class PHG4E1039InsensDetector: public PHG4Detector
{
public:
    //! constructor
    PHG4E1039InsensDetector(PHCompositeNode* node, PHParameters* parameters, const std::string& dnam = "BLOCK", const int lyr = 0);

    //! destructor
    virtual ~PHG4E1039InsensDetector(void);

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
  
public:
    double z1v, z2v, z1h, z2h;

    std::vector<G4Element*> elementVec;
    std::vector<G4Material*> materialVec;
    std::vector<G4VSolid*>  solidVec;
    std::vector<G4LogicalVolume*>  logicalVolumeVec;
    std::vector<G4RotationMatrix*> rotationMatrixVec;

    void SetTargetMaterial(std::string);
    void IronToggle(bool);
    void ReloadMagField();

private:
    G4VPhysicalVolume* physiWorld;
    int AssignAttributes(G4VPhysicalVolume*);
    MYSQL* con;
};

#endif
