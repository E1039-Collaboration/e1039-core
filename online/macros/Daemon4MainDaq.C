/// Daemon4MainDaq.C
R__LOAD_LIBRARY(libdecoder_maindaq)

bool FindExistingRuns(vector<int>& list_run)
{
  list_run.clear();
  void *dirp = gSystem->OpenDirectory(UtilOnline::GetCodaFileDir().c_str());
  if (dirp == 0) return false; // The directory does not exist.
  const char* name_char;
  while ((name_char = gSystem->GetDirEntry(dirp))) {
    string name = name_char;
    int length = name.length();
    if (length < 4 || name.substr(length-4, 4) != ".dat") continue;
    int run = UtilOnline::CodaFile2RunNum(name);
    list_run.push_back(run);
  }

  gSystem->FreeDirectory(dirp);

  sort(list_run.begin(), list_run.end());
  //cout << "Runs:";
  //for (vector<int>::iterator it = list_run.begin(); it != list_run.end(); it++) cout << " " << *it;
  //cout << endl;
  return true;
}

void StartDecoder(const int run, const int n_evt=0, const bool is_online=true)
{
  ostringstream oss;
  oss << gSystem->Getenv("E1039_CORE") << "/script/exec-decoder.sh -s -e " << n_evt;
  if (is_online) oss << " -o";
  oss << " " << run << " &";

  cout << "Start the decoder for run " << run << ":\n"
       << "  Command: " << oss.str() << endl;
  gSystem->Exec(oss.str().c_str());
}

int Daemon4MainDaq(const int interval=30)
{
  gSystem->Load("libdecoder_maindaq.so");

  vector<int> list_run_done;
  while (! FindExistingRuns(list_run_done)) {
    cout << "The Coda-file directory seems not mounted.  Wait for 1 min." << endl;
    sleep(60);
  }
  //list_run_done.clear(); // for test

  while (true) {
    cout << "Sleep for " << interval << " sec.  Hit Ctrl-C to quit..." << endl;
    sleep(interval);
    vector<int> list_run;
    if (! FindExistingRuns(list_run)) {
      cout << "The Coda-file directory doesn't exist.  Strange but just wait." << endl;
      continue;
    }
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
