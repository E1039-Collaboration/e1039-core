/*
 * Fun4kTracker.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */

#define LOCAL_DEBUG cout<<": "<<__LINE__<<endl

void Fun4kTracker(
		const int nevent = 2,
		const char *indst = "dst.root",
		const char *outeval = "eval.root"
		) {

	int verbosity = 0;

	cout<<": "<<__LINE__<<endl;

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);
	cout<<": "<<__LINE__<<endl;
	gSystem->Load("libktracker.so");
	cout<<": "<<__LINE__<<endl;
	KalmanFastTrackingWrapper *ktracker = new KalmanFastTrackingWrapper();
	cout<<": "<<__LINE__<<endl;
	se->registerSubsystem(ktracker);
	cout<<": "<<__LINE__<<endl;
	Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
	in->fileopen(indst);
	se->registerInputManager(in);
	cout<<": "<<__LINE__<<endl;
	se->run(nevent);
	se->End();
	delete se;

	cout<<": "<<__LINE__<<endl;

	cout << "Fun4TestAnalyzer Done!" << endl;
}
