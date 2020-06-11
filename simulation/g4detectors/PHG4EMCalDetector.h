#ifndef PHG4EMCalDetector_H
#define PHG4EMCalDetector_H

#include <g4main/PHG4Detector.h>

#include <cassert>
#include <map>
#include <set>
#include <string>
#include <utility>                // for pair, make_pair

class G4LogicalVolume;
class G4VPhysicalVolume;
class PHCompositeNode;
class PHG4Subsystem;
class PHG4GDMLConfig;
class PHParameters;

class PHG4EMCalDetector: public PHG4Detector
{
public:
  //! constructor
  PHG4EMCalDetector(PHCompositeNode* Node, PHParameters* parameters, const std::string& dnam = "EMCal", const int lyr = 0);

  //! destructor
  virtual ~PHG4EMCalDetector() {}

  //! construct
  virtual void Construct(G4LogicalVolume* world);

  //!@name volume accessors
  int IsInForwardEcal(G4VPhysicalVolume *) const;

  void SuperDetector(const std::string &name) { m_SuperDetector = name; }
  const std::string SuperDetector() const { return m_SuperDetector; }
  int get_Layer() const { return m_Layer; }

private:
  G4LogicalVolume* ConstructSingleTower();
  void PlaceTower(G4LogicalVolume* envelope, G4LogicalVolume* tower);

  PHParameters* m_Params;

  //! registry for volumes that should not be exported, i.e. fibers
  G4LogicalVolume* m_scintLogical;
  G4LogicalVolume* m_absorberLogical;

  int m_ActiveFlag;
  int m_AbsorberActiveFlag;
  int m_Layer;
  std::string m_SuperDetector;

  double m_tower_size_x;
  double m_tower_size_y;
  double m_tower_size_z;
  int m_ntowers_x;
  int m_ntowers_y;
  int m_npbsc_layers;


};

#endif
