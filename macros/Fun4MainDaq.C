/** Fun4MainDaq.C:  Fun4all macro to decode the MainDAQ data.
 * 
 * If using ROOT version 5, 
 * you can (need?) use "gSystem->Load()" instead of "R__LOAD_LIBRARY()".
 *
 * If using ROOT version 6, 
 * you can manually copy '*_rdict.pcm' files to suppress run-time warnings,
 * for example, by the following command;
 *   find ../build -name '*_rdict.pcm' -exec cp -p {} $MY_INSTALL/lib/ \;
 * 
 * To run this macro on a local computer, you need copy Coda file and also
 *  mapping files.  You can use the following commands;
     RUN=28700
     DIR_LOCAL=/data/e906
     
     mkdir -p $DIR_LOCAL/runs
     RUN6=$(printf '%06i' $RUN)
     scp -p  e906-gat6.fnal.gov:/data3/data/mainDAQ/run_$RUN6.dat $DIR_LOCAL
     scp -pr e906-gat6.fnal.gov:/data2/production/runs/run_$RUN6  $DIR_LOCAL/runs
 */
R__LOAD_LIBRARY(libdecoder_maindaq)

int Fun4MainDaq(
  const int nevent = 0,
  const char* fn_in  = "/data/e906/run_028700.dat",  
  const char* fn_out = "maindaq.root")
{
//  gSystem->Load("libfun4all.so");
//  gSystem->Load("libevent.so");
//  gSystem->Load("libdecoder_maindaq.so");
    
  Fun4AllServer* se = Fun4AllServer::instance();
  //se->Verbosity(1);

  //TestAnalyzer *analyzer = new TestAnalyzer();
  //analyzer->Verbosity(verbosity);
  //analyzer->set_hit_container_choice("Vector");
  //analyzer->set_out_name(outeval);
  //se->registerSubsystem(analyzer);

  Fun4AllEVIOInputManager *in = new Fun4AllEVIOInputManager("MainDaq");
  in->Verbosity(1);
  in->EventSamplingFactor(100);
  in->DirParam("/data/e906/runs");
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
