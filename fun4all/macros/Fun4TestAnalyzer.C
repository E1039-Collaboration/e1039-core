/*
 * Fun4TestAnalyzer.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */



void Fun4TestAnalyzer(
		const int nevent = 2,
		const char *infile = "dst.root",
		const char *outfile = "eval.root"
		) {

	int verbosity = 0;

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);

	gSystem->Load("libmodules.so");
	TestAnalyzer *analyzer = new TestAnalyzer();
	analyzer->Verbosity(verbosity);
	analyzer->set_hit_container_choice("Vector");
	analyzer->set_out_name(outfile);

	Fun4AllInputManager *in = new Fun4AllDstInputManager("DSTIN");
	in->fileopen(infile);
	se->registerInputManager(in);

	se->registerSubsystem(analyzer);

	se->run(nevent);
	se->End();

	delete se;

	cout << "Fun4TestAnalyzer Done!" << endl;
}
