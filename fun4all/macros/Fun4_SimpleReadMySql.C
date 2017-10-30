/*
 * Fun4_SimpleReadMySql.C
 *
 *  Created on: Oct 30, 2017
 *      Author: yuhw
 */



void Fun4_SimpleReadMySql(int nevent = 1) {

	gSystem->Load("libfun4all.so");
	Fun4AllServer* se = Fun4AllServer::instance();

	gSystem->Load("libmodules.so");
	SimpleReadMySql *simple_reader = new SimpleReadMySql();

	Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", "test.root");
	se->registerOutputManager(out);

	se->registerSubsystem(simple_reader);

	se->run(nevent);
	se->End();

	delete se;
}
