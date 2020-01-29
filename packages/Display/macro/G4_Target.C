#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <g4detectors/PHG4E1039InsensSubsystem.h>
#include <g4detectors/PHG4TargetCoilV2Subsystem.h>
#include <g4detectors/PHG4CollimatorSubsystem.h>
#include <g4detectors/PHG4CylinderSubsystem.h>
#include <g4detectors/PHG4BlockSubsystem.h>
#include <g4detectors/PHG4SquareTubeSubsystem.h>
class SubsysReco;
R__LOAD_LIBRARY(libg4detectors)
#endif

using namespace std;

void SetupTarget(
  PHG4Reco *g4Reco,
  const bool do_collimator = true,
  const bool do_target = true,
  const bool do_e1039_shielding = true,
  const double target_coil_pos_z = -300,
	const double target_l = 7.9,
	const double target_z = 0.,
	const int use_g4steps = 1
) {

  const double collimator_pos_z = target_coil_pos_z - 302.36;

  //gSystem->Load("libfun4all.so");
	//gSystem->Load("libg4detectors");
	//gSystem->Load("libg4testbench");

  //Fun4AllServer *se = Fun4AllServer::instance();
  //PHG4Reco *g4Reco = (PHG4Reco *) se->getSubsysReco("PHG4RECO");

  if(do_collimator) {
    PHG4CollimatorSubsystem* collimator = new PHG4CollimatorSubsystem("Collimator",0);
    collimator->SuperDetector("Collimator");
    collimator->set_double_param("place_z", collimator_pos_z);//-302.36 cm
    collimator->set_double_param("size_z",121.92);
    collimator->SetActive(1);
    g4Reco->registerSubsystem(collimator);
  }

  if(do_target) {
    PHG4TargetCoilV2Subsystem* coil_0 = new PHG4TargetCoilV2Subsystem("Coil", 0);
    coil_0->SuperDetector("Coil");
    coil_0->set_double_param("rot_x", 90.);
    coil_0->set_double_param("rot_y", 0.);
    coil_0->set_double_param("rot_z", 0.);
    coil_0->set_double_param("place_x", 0.);
    coil_0->set_double_param("place_y", (22.7+4.)/2);
    coil_0->set_double_param("place_z", target_coil_pos_z);
    coil_0->set_int_param("use_g4steps", use_g4steps);
    coil_0->SetActive(1);                                   // it is an active volume - save G4Hits
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
    coil_1->SetActive(1);                                   // it is an active volume - save G4Hits
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
    target->SetActive(1);                                   // it is an active volume - save G4Hits
    g4Reco->registerSubsystem(target);
  }
  
  if(do_e1039_shielding) {
    const double inch = 2.54;

    PHG4SquareTubeSubsystem* shielding = NULL;

    shielding = new PHG4SquareTubeSubsystem("Shielding1",0);
    shielding->set_string_param("hole_type","circle");
    shielding->set_double_param("place_x",0);
    shielding->set_double_param("place_y",0);
    shielding->set_double_param("place_z", (-18*inch/2.-(2.15+11.38+ 36)*inch)); // I have added all the z length and put the z into the center of mass of the block.
    shielding->set_double_param("size_x",250*inch); //the info is not given?
    shielding->set_double_param("size_y",200*inch); //the info is not given?
    shielding->set_double_param("size_z",(18-0.001)*inch);
    shielding->set_double_param("inner_diameter",4*inch);
    shielding->set_string_param("material","G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);

    shielding = new PHG4SquareTubeSubsystem("Shielding2",0);
    shielding->set_string_param("hole_type","circle");
    shielding->set_double_param("place_x",0);
    shielding->set_double_param("place_y",0);
    shielding->set_double_param("place_z",(-36*inch/2.-(2.15+11.38)*inch)); // I have added all the z length and put the z into the center of mass of the block.
    shielding->set_double_param("size_x",250*inch); //the info is not given?
    shielding->set_double_param("size_y",200*inch); //the info is not given?
    shielding->set_double_param("size_z",36*inch);
    shielding->set_double_param("inner_diameter",6*inch);
    shielding->set_string_param("material","G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);

    shielding = new PHG4SquareTubeSubsystem("Shielding3",0);
    shielding->set_double_param("place_x",0);
    shielding->set_double_param("place_y",0);
    shielding->set_double_param("place_z",-11.38*inch/2.);
    shielding->set_double_param("size_x",50*inch);
    shielding->set_double_param("size_y",50*inch);
    shielding->set_double_param("size_z",(11.38-0.001)*inch);
    shielding->set_double_param("inner_size_x",6*inch);
    shielding->set_double_param("inner_size_y",6*inch);
    shielding->set_string_param("material","G4_CONCRETE");
    g4Reco->registerSubsystem(shielding);
  }

  return;
}

