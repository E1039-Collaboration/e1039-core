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
	int verbosity = 0;
	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);
	gSystem->Load("libktracker.so");
	KalmanFastTrackingWrapper *ktracker = new KalmanFastTrackingWrapper();
  ktracker->set_geom_file_name("geom.gdml");
  ktracker->Verbosity(1);
	se->registerSubsystem(ktracker);
	Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
	in->fileopen(indst);
	se->registerInputManager(in);
	se->run(nevent);
	se->End();
  se->PrintTimer();
	delete se;

	cout << "Fun4TestAnalyzer Done!" << endl;
}
