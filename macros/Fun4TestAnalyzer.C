/*
 * Fun4TestAnalyzer.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */



void Fun4TestAnalyzer(
		const int nevent = 2,
		const char *indst = "dst.root",
		const char *outeval = "eval.root"
		) {

	int verbosity = 0;

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);

	gSystem->Load("libmodule_example.so");
	TestAnalyzer *analyzer = new TestAnalyzer();
	analyzer->Verbosity(verbosity);
	analyzer->set_hit_container_choice("Vector");
	analyzer->set_out_name(outeval);
	se->registerSubsystem(analyzer);

	Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
	in->fileopen(indst);
	se->registerInputManager(in);

	se->run(nevent);
	se->End();

	delete se;

	cout << "Fun4TestAnalyzer Done!" << endl;
}
