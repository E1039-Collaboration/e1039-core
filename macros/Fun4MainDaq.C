/** Fun4MainDaq.C:  Fun4all macro to decode the MainDAQ data.
 * 
 * 
 */
R__LOAD_LIBRARY(libdecoder_maindaq)

int Fun4MainDaq(
  const int nevent = 10000,
  const char* fn_in  = "/data/e906/run_028700.dat",  
  const char* fn_out = "dst.root")
{
  int verbosity = 1;
  
//  gSystem->Load("libfun4all.so");
//  gSystem->Load("libevent.so");
//  gSystem->Load("libdecoder_maindaq.so");
//    
  Fun4AllServer* se = Fun4AllServer::instance();
  se->Verbosity(verbosity);

  //ReadMySql *reader = new ReadMySql();
  //reader->Verbosity(verbosity);
  //reader->set_hit_type("SQHit_v1");
  //reader->set_hit_container_type("Vector");
  //se->registerSubsystem(reader);
  //
  //TestAnalyzer *analyzer = new TestAnalyzer();
  //analyzer->Verbosity(verbosity);
  //analyzer->set_hit_container_choice("Vector");
  //analyzer->set_out_name(outeval);
  //se->registerSubsystem(analyzer);

  Fun4AllInputManager *in = new Fun4AllEVIOInputManager("MainDaq");
  in->Verbosity(verbosity);
  in->fileopen(fn_in);
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", fn_out);
  se->registerOutputManager(out);

  se->run(nevent);
  se->End();
  
  delete se;
  cout << "Fun4MainDaq Done!" << endl;
  return 0;
}
