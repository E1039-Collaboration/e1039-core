#ifndef PHG4BNLTargetCoilDetector_h
#define PHG4BNLTargetCoilDetector_h

#include <g4main/PHG4Detector.h>

#include <string>

class G4LogicalVolume;
class G4VPhysicalVolume;
class PHParameters;

class PHG4BNLTargetCoilDetector : public PHG4Detector
{
 public:
  //! constructor
  PHG4BNLTargetCoilDetector(PHCompositeNode *Node, PHParameters *parameters, const std::string &dnam, const int layer = 0);

  //! destructor
  virtual ~PHG4BNLTargetCoilDetector(void)
  {
  }

  //! construct
  void Construct(G4LogicalVolume *world);

  bool IsInCylinder(const G4VPhysicalVolume *) const;
  void SuperDetector(const std::string &name) { superdetector = name; }
  const std::string SuperDetector() const { return superdetector; }
  int get_Layer() const { return layer; }
  int get_elem_id(const std::string &name) const;
 private:
  PHParameters *params;

  G4VPhysicalVolume *cylinder_physi;

  int layer;
  std::string superdetector;
};

#endif
