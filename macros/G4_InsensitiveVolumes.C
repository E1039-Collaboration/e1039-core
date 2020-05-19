#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4BlockSubsystem.h>
#include <g4detectors/SQG4DipoleMagnetSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#endif

void SetupInsensitiveVolumes(
  PHG4Reco* g4Reco,
  const int toggle_fmag = 1,
  const int toggle_kmag = 1,
  const int toggle_absorber = 1,
  const int enable_fmag_filter = 0,   /*should set this to 1 after being extensively tested*/
  const double filter_max_slope = 0.25,
  const double filter_min_energy = 5.)
{
  if(toggle_fmag == 1) {
    SQG4DipoleMagnetSubsystem* fmag = new SQG4DipoleMagnetSubsystem("fmag");
    fmag->set_string_param("geomdb", "$E1039_RESOURCE/geometry/magnetic_fields/magnet_geom.db");
    fmag->set_string_param("magname", "fmag");
    fmag->set_double_param("place_x", 0.0);
    fmag->set_double_param("place_y", 0.0);
    fmag->set_double_param("place_z", 251.46);
    if(enable_fmag_filter == 1) {
      fmag->set_int_param("enable_track_filter", 1);
      fmag->set_double_param("filter_max_slope", filter_max_slope);
      fmag->set_double_param("filter_min_energy", filter_min_energy);
    }

    g4Reco->registerSubsystem(fmag);
  }

  if(toggle_kmag == 1) {
    SQG4DipoleMagnetSubsystem* kmag = new SQG4DipoleMagnetSubsystem("fmag");
    kmag->set_string_param("geomdb", "$E1039_RESOURCE/geometry/magnetic_fields/magnet_geom.db");
    kmag->set_string_param("magname", "kmag");
    kmag->set_double_param("place_x", -1.34493);
    kmag->set_double_param("place_y", -1.0414);
    kmag->set_double_param("place_z", 1042.01722);

    g4Reco->registerSubsystem(kmag);
  }

  if(toggle_absorber == 1) {
    PHG4BlockSubsystem* absorber = new PHG4BlockSubsystem("MUID_absorber", 0);
    absorber->SuperDetector("MUID_absorber");
    absorber->set_double_param("size_x", 320.04);
    absorber->set_double_param("size_y", 345.44);
    absorber->set_double_param("size_z", 99.568);
    absorber->set_double_param("place_x", 0.);
    absorber->set_double_param("place_y", 0.);
    absorber->set_double_param("place_z", 2028.19);
    absorber->set_double_param("rot_x", 0.);
    absorber->set_double_param("rot_y", 0.);
    absorber->set_double_param("rot_z", 0.);
    absorber->set_string_param("material", "G4_Fe");
    absorber->SetActive(0);

    g4Reco->registerSubsystem(absorber);
  }

  return;
}
