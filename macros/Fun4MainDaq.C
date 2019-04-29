/** Fun4MainDaq.C:  Fun4all macro to decode the MainDAQ data.
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
R__LOAD_LIBRARY(libinterface_main)
R__LOAD_LIBRARY(libonlmonserver)
R__LOAD_LIBRARY(libdecoder_maindaq)

int Fun4MainDaq(const int nevent = 0, const int run = 28700)
{
  //gSystem->Load("libdecoder_maindaq.so");
  //const char* dir_in  = "/data/e906",
  const char* dir_in  = "/seaquest/analysis/kenichi/e1039";
  const char* dir_out = ".";
  const bool is_online = false; // true;

  ostringstream oss;
  oss << setfill('0') 
      << dir_in << "/run_" << setw(6) << run << ".dat";
  string fn_in = oss.str();
  oss.str("");
  oss << dir_out << "/run_" << setw(6) << run << ".root";
  string fn_out = oss.str();

  //Fun4AllServer* se = Fun4AllServer::instance();
  OnlMonServer* se = OnlMonServer::instance();
  //se->Verbosity(1);

  Fun4AllEVIOInputManager *in = new Fun4AllEVIOInputManager("MainDaq");
  in->Verbosity(1);
  in->EventSamplingFactor(100);
  if (is_online) {
    in->PretendSpillInterval(55);
  }
  in->DirParam("/seaquest/production/runs");
  //in->DirParam("/data/e906/runs");
  in->fileopen(fn_in);
  se->registerInputManager(in);

  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", fn_out);
  se->registerOutputManager(out);

  se->registerSubsystem(new CalibInTime());
  se->registerSubsystem(new CalibXT());

  if (is_online) { // Register the online-monitoring clients
    se->StartServer();

    OnlMonClient* ana = new OnlMonMainDaq();
    se->registerSubsystem(ana);
  }

  se->run(nevent);
  se->End();
  
  delete se;
  cout << "Fun4MainDaq Done!" << endl;
  return 0;
}
