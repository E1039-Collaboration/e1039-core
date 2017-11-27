/*
 * Fun4ReadMySql.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */



void Fun4ReadMySql(
		const int nevent = 2, 
		const char *outfile = "out.root"
		) {

	int verbosity = 0;

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);

	gSystem->Load("libmodules.so");
	ReadMySql *simple_reader = new ReadMySql();
	simple_reader->Verbosity(verbosity);
	simple_reader->set_hit_container_choice("Vector");

	Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
	se->registerInputManager(in);

	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfile);
	se->registerOutputManager(out);

	se->registerSubsystem(simple_reader);

	se->run(nevent);
	se->End();

	delete se;

	cout << "Fun4ReadMySql Done!" << endl;
}
