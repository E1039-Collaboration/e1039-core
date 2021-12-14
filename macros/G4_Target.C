/** @file
 * @brief Macro to configure the target objects.
 *
 * The usage is similar to `G4_SensitiveDetectors.C`.
 */
#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4TargetCoilV2Subsystem.h>
#include <g4detectors/PHG4CylinderSubsystem.h>
#include <g4detectors/PHG4BlockSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#endif

/// Macro function to configure the target objects.
/**
 * t.b.w.
 */
void SetupTarget(
  PHG4Reco* g4Reco,
  const double target_coil_pos_z = -300,
  const double target_l = 7.9,
  const double target_z = 0.,
  const int use_g4steps = 1,
  const int register_hits = 0) 
{
  PHG4TargetCoilV2Subsystem* coil_0 = new PHG4TargetCoilV2Subsystem("Coil", 0);
  coil_0->SuperDetector("Coil");
  coil_0->set_double_param("rot_x", 90.);
  coil_0->set_double_param("rot_y", 0.);
  coil_0->set_double_param("rot_z", 0.);
  coil_0->set_double_param("place_x", 0.);
  coil_0->set_double_param("place_y", (22.7+4.)/2);
  coil_0->set_double_param("place_z", target_coil_pos_z);
  coil_0->set_int_param("use_g4steps", use_g4steps);
  coil_0->SetActive(register_hits);
  g4Reco->registerSubsystem(coil_0);

  PHG4TargetCoilV2Subsystem* coil_1 = new PHG4TargetCoilV2Subsystem("Coil", 1);
  coil_1->SuperDetector("Coil");
  coil_1->set_double_param("rot_x", -90.);
  coil_1->set_double_param("rot_y", 0.);
  coil_1->set_double_param("rot_z", 0.);
  coil_1->set_double_param("place_x", 0.);
  coil_1->set_double_param("place_y", -(22.7+4.)/2);
  coil_1->set_double_param("place_z", target_coil_pos_z);
  coil_1->set_int_param("use_g4steps", use_g4steps);
  coil_1->SetActive(register_hits);
  g4Reco->registerSubsystem(coil_1);

  PHG4CylinderSubsystem* target = new PHG4CylinderSubsystem("Target", 0);
  target->SuperDetector("Target");
  target->set_double_param("length", target_l);
  target->set_double_param("rot_x", 0.);
  target->set_double_param("rot_y", 0.);
  target->set_double_param("rot_z", 0.);
  target->set_double_param("place_x", 0.);
  target->set_double_param("place_y", 0.);
  target->set_double_param("place_z", target_coil_pos_z + target_z);
  target->set_double_param("radius", 0.);
  target->set_double_param("thickness", (2.)/2);
  target->set_string_param("material", "Target");          // material of target
  target->set_int_param("lengthviarapidity", 0);
  target->set_int_param("use_g4steps", use_g4steps);
  target->SetActive(register_hits);
  g4Reco->registerSubsystem(target);
  
  return;
}

