
#include <iostream>

using namespace std;

int Fun4AllSim(const int nEvents = 1)
{
	const int use_g4steps = 1;
	const double target_l = 7.9; //cm
	const double target_z = (7.9-target_l)/2.; //cm

	gSystem->Load("libfun4all");
	gSystem->Load("libg4detectors");
	gSystem->Load("libg4testbench");
	gSystem->Load("libg4eval");
	gSystem->Load("libtruth_eval.so");

	///////////////////////////////////////////
	// Make the Server
	//////////////////////////////////////////
	Fun4AllServer *se = Fun4AllServer::instance();
	se->Verbosity(1);

	// particle gun
	PHG4ParticleGun *gun = new PHG4ParticleGun("PGUN");
	gun->set_name("proton");//proton
	gun->set_vtx(0, 0, -400); //-363.32 cm
	gun->set_mom(0, 0, 120);
	TF2 *beam_profile = new TF2("beam_profile",
			//"(((x**2+y**2)<=0.81)*exp(-(x**2+y**2)/0.18))+(((x**2+y**2)>0.81&&(x**2+y**2)<=25&&abs(y)<1.)*0.9*exp(-4.5)/(sqrt(x**2+y**2)))",
			"(((x**2+y**2)<=0.81)*exp(-(x**2+y**2)/0.18))+(((x**2+y**2)>0.81&&(x**2+y**2)<=25)*0.9*exp(-4.5)/(sqrt(x**2+y**2)))",
			-5,5,-5,5);
	gun->set_beam_profile(beam_profile);
	se->registerSubsystem(gun);

	// Fun4All G4 module
	PHG4Reco *g4Reco = new PHG4Reco();
	//g4Reco->G4Seed(123);
	//g4Reco->set_field(5.);
	g4Reco->set_field_map("target.field.root",4);
	// size of the world - every detector has to fit in here
	g4Reco->SetWorldSizeX(100);
	g4Reco->SetWorldSizeY(100);
	g4Reco->SetWorldSizeZ(800);
	// shape of our world - it is a tube
	g4Reco->SetWorldShape("G4BOX");
	// this is what our world is filled with
	g4Reco->SetWorldMaterial("G4_Galactic");
	// Geant4 Physics list to use
	g4Reco->SetPhysicsList("FTFP_BERT");

	PHG4CollimatorSubsystem* collimator = new PHG4CollimatorSubsystem("Collimator",0);
	collimator->SuperDetector("Collimator");
	collimator->set_double_param("place_z",-302.36);//-302.36 cm
	collimator->set_double_param("size_z",121.92);
	collimator->SetActive(1);
	g4Reco->registerSubsystem(collimator);

	PHG4TargetCoilSubsystem* coil_0 = new PHG4TargetCoilSubsystem("Coil", 0);
	coil_0->SuperDetector("Coil");
	coil_0->set_double_param("rot_x", 90.);
	coil_0->set_double_param("rot_y", 0.);
	coil_0->set_double_param("rot_z", 0.);
	coil_0->set_double_param("place_x", 0.);
	coil_0->set_double_param("place_y", (22.7+4.)/2);
	coil_0->set_double_param("place_z", 0.);
	coil_0->set_int_param("use_g4steps", use_g4steps);
	coil_0->SetActive(1);                                   // it is an active volume - save G4Hits
	g4Reco->registerSubsystem(coil_0);

	PHG4TargetCoilSubsystem* coil_1 = new PHG4TargetCoilSubsystem("Coil", 1);
	coil_1->SuperDetector("Coil");
	coil_1->set_double_param("rot_x", -90.);
	coil_1->set_double_param("rot_y", 0.);
	coil_1->set_double_param("rot_z", 0.);
	coil_1->set_double_param("place_x", 0.);
	coil_1->set_double_param("place_y", -(22.7+4.)/2);
	coil_1->set_double_param("place_z", 0.);
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
	target->set_double_param("place_z", target_z);
	target->set_double_param("radius", 0.);
	target->set_double_param("thickness", (2.)/2);
	target->set_string_param("material", "Target");          // material of target
	target->set_int_param("lengthviarapidity", 0);
	target->set_int_param("use_g4steps", use_g4steps);
	target->SetActive(1);                                   // it is an active volume - save G4Hits
	g4Reco->registerSubsystem(target);


	PHG4TruthSubsystem *truth = new PHG4TruthSubsystem();
	g4Reco->registerSubsystem(truth);

	se->registerSubsystem(g4Reco);

	TruthEval* eval = new TruthEval("TruthEval","eval.root");
	eval->target_l = target_l;
	eval->target_z = target_z;
	se->registerSubsystem(eval);

	///////////////////////////////////////////
	// Output
	///////////////////////////////////////////

	// save a comprehensive  evaluation file
	PHG4DSTReader *ana = new PHG4DSTReader(
			string("DSTReader.root"));
	ana->set_save_particle(true);
	ana->set_load_all_particle(false);
	ana->set_load_active_particle(true);
	ana->set_save_vertex(true);
	ana->AddNode("Coil");
	ana->AddNode("Target");
	ana->AddNode("Collimator");
	se->registerSubsystem(ana);

	// input - we need a dummy to drive the event loop
	Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
	se->registerInputManager(in);

	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "DST.root");
	se->registerOutputManager(out);

	// a quick evaluator to inspect on hit/particle/tower level

	if (nEvents > 0)
	{
		se->run(nEvents);

		PHGeomUtility::ExportGeomtry(se->topNode(),"geom.root");

		// finish job - close and save output files
		se->End();
		std::cout << "All done" << std::endl;

		// cleanup - delete the server and exit
		delete se;
		gSystem->Exit(0);
	}
	return;
}

PHG4ParticleGun *get_gun(const char *name = "PGUN")
{
	Fun4AllServer *se = Fun4AllServer::instance();
	PHG4ParticleGun *pgun = (PHG4ParticleGun *) se->getSubsysReco(name);
	return pgun;
}
