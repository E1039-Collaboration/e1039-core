/*
 * Fun4ReadMySql.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw@nmsu.edu
 */



void Fun4ReadMySql(
		const int nevent = 2, 
		const char *outdst = "dst.root"
		) {

	int verbosity = 0;

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();
	se->Verbosity(verbosity);

	gSystem->Load("libmodules.so");
	ReadMySql *reader = new ReadMySql();
	reader->Verbosity(verbosity);
	reader->set_hit_type("SQHit_v1");
	reader->set_hit_container_type("Vector");
	se->registerSubsystem(reader);


	Fun4AllInputManager *in = new Fun4AllDummyInputManager("DUMMY");
	se->registerInputManager(in);

	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outdst);
	se->registerOutputManager(out);


	se->run(nevent);
	se->End();

	delete se;

	cout << "Fun4ReadMySql Done!" << endl;
}
