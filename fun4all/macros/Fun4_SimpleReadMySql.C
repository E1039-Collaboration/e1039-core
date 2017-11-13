/*
 * Fun4_SimpleReadMySql.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw
 */



void Fun4_SimpleReadMySql(
const int nevent = 1, 
const char *outfile = "out.root"
) {

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();

	gSystem->Load("libmodules.so");
	SimpleReadMySql *simple_reader = new SimpleReadMySql();
    simple_reader->Verbosity(100);

    Fun4AllInputManager *in = new Fun4AllDummyInputManager("JADE");
	se->registerInputManager(in);

	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", outfile);
	se->registerOutputManager(out);

	se->registerSubsystem(simple_reader);

	se->run(nevent);
	se->End();

	delete se;
}
