#ifndef PHG4EMCalSubsystem_H
#define PHG4EMCalSubsystem_H

#include "PHG4DetectorSubsystem.h"

#include <string>  // for string

class PHCompositeNode;
class PHG4Detector;
class PHG4DisplayAction;
class PHG4EMCalDetector;
class PHG4SteppingAction;

class PHG4EMCalSubsystem: public PHG4DetectorSubsystem
{
public:
  /** Constructor
  */
  PHG4EMCalSubsystem(const std::string& name = "EMCal", const int layer = 0);

  /** Destructor
  */
  virtual ~PHG4EMCalSubsystem();

  /**
  Creates the m_Detector object and place it on the node tree, under "DETECTORS" node (or whatever)
  Creates the stepping action and place it on the node tree, under "ACTIONS" node
  Creates relevant hit nodes that will be populated by the stepping action and stored in the output DST
  */
  int InitRunSubsystem(PHCompositeNode*);

  /** Event processing
  */
  int process_event(PHCompositeNode*);

  /** Accessors (reimplemented)
  */
  PHG4Detector* GetDetector() const;
  PHG4SteppingAction* GetSteppingAction() const { return m_SteppingAction; }

private:
  void SetDefaultParameters();

  /** Pointer to the Geant4 implementation of the detector
  */
  PHG4EMCalDetector* m_Detector;

  /** Stepping action
  */
  PHG4SteppingAction* m_SteppingAction;
};

#endif
