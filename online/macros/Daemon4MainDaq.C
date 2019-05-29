/// Daemon4MainDaq.C
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
R__LOAD_LIBRARY(libdecoder_maindaq)
#endif

void FindExistingRuns(vector<int>& list_run)
{
  list_run.clear();
  void *dirp = gSystem->OpenDirectory(UtilOnline::GetCodaFileDir().c_str());
  const char* name_char;
  while ((name_char = gSystem->GetDirEntry(dirp))) {
    string name = name_char;
    int length = name.length();
    if (length < 4 || name.substr(length-4, 4) != ".dat") continue;
    int run = UtilOnline::CodaFile2RunNum(name);
    //cout << "name = " << name << " " << run << endl;
    list_run.push_back(run);
  }
}

void StartDecoder(const int run)
{
  ostringstream oss;
  oss << "/dev/shm/decoder_maindaq";
  gSystem->mkdir(oss.str().c_str(), true);
  oss << "/log_" << setfill('0') << setw(6) << run << ".txt";
  string fn_log = oss.str();
  oss.str("");
  oss << "root -b -q '" << gSystem->Getenv("E1039_CORE_SRC")
      << "/online/macros/Fun4MainDaq.C(" << run << ", 0, true)' &>" << fn_log << " &";

  cout << "Start the decoder for run " << run << ":\n"
       << "  Log file = " << fn_log << "\n"
       << "  Command: " << oss.str() << endl;
  gSystem->Exec(oss.str().c_str());
}

int Daemon4MainDaq(const string mode="daemon")
{
  gSystem->Load("libdecoder_maindaq.so");

  vector<int> list_run_done;
  FindExistingRuns(list_run_done);
  //list_run_done.clear(); // for test

  while (true) {
    cout << "Sleep for 30 sec.  Hit Ctrl-C to quit..." << endl;
    sleep(30);
    vector<int> list_run;
    FindExistingRuns(list_run);
    for (vector<int>::iterator it = list_run.begin(); it != list_run.end(); it++) {
      int run = *it;
      if (find(list_run_done.begin(), list_run_done.end(), run) == list_run_done.end()) {
        list_run_done.push_back(run);
        StartDecoder(run);
      }
    }
  }

  return 0;
}
