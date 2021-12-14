/** @file
 * @brief Macro to configure the beamline objects.
 *
 * The usage is similar to `G4_SensitiveDetectors.C`.
 */
#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4CollimatorSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#endif

/// Macro function to configure the beamline objects.
/**
 * t.b.w.
 */
void SetupBeamline(
  PHG4Reco* g4Reco,
  const bool toggle_collimator = true,
  const double collimator_pos_z = -602.36,
  const int register_hits = 0) 
{
  if(toggle_collimator) {
    PHG4CollimatorSubsystem* collimator = new PHG4CollimatorSubsystem("Collimator",0);
    collimator->SuperDetector("Collimator");
    collimator->set_double_param("place_z", collimator_pos_z);
    collimator->set_double_param("size_z", 121.92);
    collimator->SetActive(register_hits);
    g4Reco->registerSubsystem(collimator);
  }
 
  return;
}

