#include "SQG4DipoleMagnetSubsystem.h"
#include "SQG4DipoleMagnetDetector.h"
#include "PHG4BlockGeomContainer.h"
#include "PHG4BlockGeomv1.h"
#include "SQG4DipoleMagnetSteppingAction.h"

#include <phparameter/PHParameters.h>
#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Utils.h>
#include <phool/getClass.h>

#include <Geant4/globals.hh>
#include <sstream>

//_______________________________________________________________________
SQG4DipoleMagnetSubsystem::SQG4DipoleMagnetSubsystem(const std::string& name, const int lyr)
  : PHG4DetectorSubsystem(name, lyr)
  , _detector(nullptr)
  , _steppingAction(nullptr)
{
  InitializeParameters();
}

//_______________________________________________________________________
int SQG4DipoleMagnetSubsystem::InitRunSubsystem(PHCompositeNode* topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));

  // create detector
  _detector = new SQG4DipoleMagnetDetector(topNode, GetParams(), Name(), GetLayer());
  _detector->SuperDetector(SuperDetector());
  _detector->OverlapCheck(CheckOverlap());
  if(GetParams()->get_int_param("enable_track_filter"))
  {
    _steppingAction = new SQG4DipoleMagnetSteppingAction(_detector, GetParams());
  }

  return 0;
}

//_______________________________________________________________________
int SQG4DipoleMagnetSubsystem::process_event(PHCompositeNode *topNode)
{
  // pass top node to stepping action so that it gets
  // relevant nodes needed internally
  if(_steppingAction != nullptr)
  {
    _steppingAction->SetInterfacePointers(topNode);
  }
  return 0;
}

//_______________________________________________________________________
PHG4Detector* SQG4DipoleMagnetSubsystem::GetDetector(void) const
{
  return _detector;
}

void SQG4DipoleMagnetSubsystem::SetDefaultParameters()
{
  set_default_double_param("place_x", 0.);
  set_default_double_param("place_y", 0.);
  set_default_double_param("place_z", 0.);
  set_default_double_param("rot_x", 0.);
  set_default_double_param("rot_y", 0.);
  set_default_double_param("rot_z", 0.);
  set_default_double_param("steplimits", NAN);
  set_default_double_param("size_x", 10.);
  set_default_double_param("size_y", 10.);
  set_default_double_param("size_z", 10.);
  set_default_int_param("use_g4steps", 0);
  set_default_string_param("material", "G4_Galactic");

  set_default_string_param("geomdb", "");
  set_default_string_param("magname", "");
  set_default_int_param("enable_track_filter", 0);
  set_default_double_param("filter_max_slope", 10.);
  set_default_double_param("filter_min_energy", 0.);
}
