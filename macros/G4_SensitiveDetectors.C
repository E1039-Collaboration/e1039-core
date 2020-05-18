#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4BlockSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#include <iostream>
#endif

#define LogDebug(exp)   std::cout<<"DEBUG: "  <<__FILE__<<": "<<__LINE__<<": "<< exp << std::endl

void SetupSensitiveDetectors(
  PHG4Reco* g4Reco,
  const int verbosity = 0)
{
  using namespace std;
  GeomSvc* geom_svc = GeomSvc::instance();

  LogDebug("");
  vector<string> sim_list = geom_svc->getDefaultSimList();
  for(int i=0; i<sim_list.size(); ++i) {
    LogDebug(i);
    string name = sim_list[i];

    double size[3];
    double place[3];
    double rot[3];

    LogDebug("");

    int id = geom_svc->getDetectorID(name);
    place[0] = geom_svc->getDetectorX0(name);
    place[1] = geom_svc->getDetectorY0(name);
    place[2] = geom_svc->getDetectorZ0(name);
    size[0] = geom_svc->getPlaneScaleX(id);
    size[1] = geom_svc->getPlaneScaleY(id);
    size[2] = geom_svc->getPlaneScaleZ(id);
    string material = geom_svc->getPlaneMaterial(id);

    if(verbosity > 2) {
      LogDebug("");
      cout
        << "name: " << name
        << ", id: " << id
        << " {" << size[0] << ", " << size[1] << ", " << size[2] << "} "
        << " {" << place[0] << ", " << place[1] << ", " << place[2] << "} "
        << " {" << rot[0] << ", " << rot[1] << ", " << rot[2] << "} "
        << material
        << endl;
    }

    if(!(fabs(size[0])<10000)) continue;
    if(place[2]>680 && place[2]<700) continue;

    PHG4BlockSubsystem* box = new PHG4BlockSubsystem(name.c_str(), 0);
    box->SuperDetector(name.c_str());
    box->set_double_param("size_x", size[0]);
    box->set_double_param("size_y", size[1]);
    box->set_double_param("size_z", size[2]);
    box->set_double_param("place_x", place[0]);
    box->set_double_param("place_y", place[1]);
    box->set_double_param("place_z", place[2]);
    box->set_string_param("material", material.c_str());// G4_Si, G4_AIR, G4_Galactic
    box->SetActive(1);
    g4Reco->registerSubsystem(box);
  }

  LogDebug("");

  return;
}
