#include "PHG4EMCalSubsystem.h"
#include "PHG4EMCalDetector.h"
#include "PHG4EMCalSteppingAction.h"

#include <phparameter/PHParameters.h>

#include <g4main/PHG4HitContainer.h>
#include <g4main/PHG4Utils.h>

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>

#include <cstdlib>                         // for getenv
#include <set>  // for set
#include <sstream>

class PHG4Detector;

//_______________________________________________________________________
PHG4EMCalSubsystem::PHG4EMCalSubsystem(const std::string& name, const int lyr)
  : PHG4DetectorSubsystem(name, lyr)
  , m_Detector(nullptr)
  , m_SteppingAction(nullptr)
{
  InitializeParameters();
}

//_______________________________________________________________________
PHG4EMCalSubsystem::~PHG4EMCalSubsystem()
{
}

//_______________________________________________________________________
int PHG4EMCalSubsystem::InitRunSubsystem(PHCompositeNode* topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

  // create detector
  m_Detector = new PHG4EMCalDetector(topNode, GetParams(), Name());

  m_Detector->SuperDetector(SuperDetector());
  m_Detector->OverlapCheck(CheckOverlap());
  m_Detector->Verbosity(Verbosity());

  std::set<std::string> nodes;
  if(GetParams()->get_int_param("active") > 0)
  {
    std::ostringstream nodename;
    if(SuperDetector() != "NONE")
    {
      nodename << "G4HIT_" << SuperDetector();
    }
    else
    {
      nodename << "G4HIT_" << Name();
    }
    nodes.insert(nodename.str());

    if(GetParams()->get_int_param("absorberactive") > 0)
    {
      nodename.str("");
      if(SuperDetector() != "NONE")
      {
        nodename << "G4HIT_ABSORBER_" << SuperDetector();
      }
      else
      {
        nodename << "G4HIT_ABSORBER_" << Name();
      }
      nodes.insert(nodename.str());
    }

    for(auto nodename: nodes)
    {
      PHG4HitContainer* g4_hits = findNode::getClass<PHG4HitContainer>(topNode, nodename);
      if(!g4_hits)
      {
        g4_hits = new PHG4HitContainer(nodename);
        dstNode->addNode(new PHIODataNode<PHObject>(g4_hits, nodename, "PHObject"));
      }
    }

    // create stepping action
    m_SteppingAction = new PHG4EMCalSteppingAction(m_Detector, GetParams());
  }

  return 0;
}

//_______________________________________________________________________
int PHG4EMCalSubsystem::process_event(PHCompositeNode* topNode)
{
  // pass top node to stepping action so that it gets
  // relevant nodes needed internally
  if(m_SteppingAction)
  {
    m_SteppingAction->SetInterfacePointers(topNode);
  }
  return 0;
}

PHG4Detector* PHG4EMCalSubsystem::GetDetector(void) const
{ 
  return m_Detector;
}

void PHG4EMCalSubsystem::SetDefaultParameters()
{
  set_default_double_param("place_x", 0.);
  set_default_double_param("place_y", 0.);
  set_default_double_param("place_z", 0.);
  set_default_double_param("rot_x", 0.);
  set_default_double_param("rot_y", 0.);
  set_default_double_param("rot_z", 0.);

  set_default_int_param("n_towers_x", 6*6*2);
  set_default_int_param("n_towers_y", 3*6*2);

  set_default_double_param("tower_x", 5.535);
  set_default_double_param("tower_y", 5.535);
  set_default_double_param("tower_z", 0.56*66);
  set_default_int_param("n_layers", 66);

  set_int_param("absorberactive", 0);
  set_int_param("active", 1);
}

