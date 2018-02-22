/*
 * Fun4kTracker.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */

void Fun4kTracker(
		const int nevent = 2,
		const char *indst = "dst.root",
		const char *outeval = "eval.root"
		) {
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	int verbosity = 0;
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	gSystem->Load("libfun4all.so");
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	Fun4AllServer* se = Fun4AllServer::instance();
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	se->Verbosity(verbosity);
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	gSystem->Load("libktracker.so");
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	KalmanFastTrackingWrapper *ktracker = new KalmanFastTrackingWrapper();
  ktracker->set_geom_file_name("geom.gdml");
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	se->registerSubsystem(ktracker);
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	in->fileopen(indst);
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	se->registerInputManager(in);
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	se->run(nevent);
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	se->End();
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;
	delete se;
	std::cout<<"DEBUG: "<<__LINE__<<std::endl;

	cout << "Fun4TestAnalyzer Done!" << endl;
}
