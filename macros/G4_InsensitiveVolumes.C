/** @file
 * @brief Macro to configure the insensitive volumes.
 *
 * The usage is similar to `G4_SensitiveDetectors.C`.
 */
#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4BlockSubsystem.h>
#include <g4detectors/PHG4SquareTubeSubsystem.h>
#include <g4detectors/SQG4DipoleMagnetSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#endif

/// Macro function to configure the insensitive volumes.
/**
 * This macro implements the all the volumes that are not sensitive - i.e. do not need to generate G4Hits.
 * The insensitive volumes implemented here are:
 *     1. concrete shielding in front of FMag
 *     2. FMag/beam dump
 *     3. KMag
 *     4. Muon absorber between station-3 and -4
 */
void SetupInsensitiveVolumes(
  PHG4Reco* g4Reco,
  const bool toggle_shielding = true,
  const bool toggle_fmag = true,
  const bool toggle_kmag = true,
  const bool toggle_absorber = true,
  const int enable_fmag_filter = 0,      /*should set this to 1 after being extensively tested*/
  const double filter_max_slope = 0.25,
  const double filter_min_energy = 5.)
{
  if(toggle_shielding) {
    const double inch = 2.54;
    PHG4SquareTubeSubsystem* shielding = nullptr;

    shielding = new PHG4SquareTubeSubsystem("Shielding1", 0);
    shielding->set_string_param("hole_type", "circle");
    shielding->set_double_param("place_x", 0);
    shielding->set_double_param("place_y", 0);
    shielding->set_double_param("place_z", (-18.*inch/2.-(2.15+11.38+36.)*inch)); // I have added all the z length and put the z into the center of mass of the block.
    shielding->set_double_param("size_x", 250.*inch); //the info is not given?
    shielding->set_double_param("size_y", 200.*inch); //the info is not given?
    shielding->set_double_param("size_z", (18.-0.001)*inch);
    shielding->set_double_param("inner_diameter", 4.*inch);
    shielding->set_string_param("material", "G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);

    shielding = new PHG4SquareTubeSubsystem("Shielding2", 0);
    shielding->set_string_param("hole_type", "circle");
    shielding->set_double_param("place_x", 0.);
    shielding->set_double_param("place_y", 0.);
    shielding->set_double_param("place_z", (-36.*inch/2.-(2.15+11.38)*inch)); // I have added all the z length and put the z into the center of mass of the block.
    shielding->set_double_param("size_x", 250.*inch); //the info is not given?
    shielding->set_double_param("size_y", 200.*inch); //the info is not given?
    shielding->set_double_param("size_z", 36.*inch);
    shielding->set_double_param("inner_diameter", 6.*inch);
    shielding->set_string_param("material", "G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);

    shielding = new PHG4SquareTubeSubsystem("Shielding3", 0);
    shielding->set_double_param("place_x", 0.);
    shielding->set_double_param("place_y", 0.);
    shielding->set_double_param("place_z", -11.38*inch/2.);
    shielding->set_double_param("size_x", 50.*inch);
    shielding->set_double_param("size_y", 50.*inch);
    shielding->set_double_param("size_z", (11.38-0.001)*inch);
    shielding->set_double_param("inner_size_x", 6.*inch);
    shielding->set_double_param("inner_size_y", 6.*inch);
    shielding->set_string_param("material", "G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);
  }

  if(false) { // false by default at present.  Change to true manually when necessary
    const double inch = 2.54;
    PHG4BlockSubsystem* shielding_behind_fmag = new PHG4BlockSubsystem("ShieldingBehindFmag", 0);
    shielding_behind_fmag->set_double_param("place_x",   0.0); // Place and size are preliminary.  See DocDB 9732
    shielding_behind_fmag->set_double_param("place_y", -6.*inch);
    shielding_behind_fmag->set_double_param("place_z", 562.0);
    shielding_behind_fmag->set_double_param("size_x", 144.*inch);
    shielding_behind_fmag->set_double_param("size_y", 144.*inch);
    shielding_behind_fmag->set_double_param("size_z",  18.*inch);
    shielding_behind_fmag->set_string_param("material", "G4_CONCRETE");
    g4Reco->registerSubsystem(shielding_behind_fmag);
  }

  if(toggle_fmag) {
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

  if(toggle_kmag) {
    SQG4DipoleMagnetSubsystem* kmag = new SQG4DipoleMagnetSubsystem("kmag");
    kmag->set_string_param("geomdb", "$E1039_RESOURCE/geometry/magnetic_fields/magnet_geom.db");
    kmag->set_string_param("magname", "kmag");
    kmag->set_double_param("place_x", -1.34493);
    kmag->set_double_param("place_y", -1.0414);
    kmag->set_double_param("place_z", 1042.01722);

    g4Reco->registerSubsystem(kmag);
  }

  if(toggle_absorber) {
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
