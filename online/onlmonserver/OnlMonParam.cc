#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <TROOT.h>
#include <TSystem.h>
#include <phool/recoConsts.h>
#include "OnlMonClient.h"
#include "OnlMonParam.h"
using namespace std;

OnlMonParam::OnlMonParam(const OnlMonClient* omc)
  : m_name(omc->Name())
  , m_dir_base("$E1039_ROOT/onlmon/param")
  , m_run_id(0)
  , m_verb(2) // High default verbosity for now
  , m_do_assert(true)
{
  ;
}

OnlMonParam::OnlMonParam(const std::string name)
  : m_name(name)
  , m_dir_base("$E1039_ROOT/onlmon/param")
  , m_run_id(0)
  , m_verb(2) // High default verbosity for now
  , m_do_assert(true)
{
  ;
}

OnlMonParam::~OnlMonParam()
{
  ;
}

std::string OnlMonParam::GetCharParam(const std::string par_name)
{
  string value = FindParamInDir(par_name);
  if (m_verb > 0) cout << "  par_value = " << value << " as string." << endl;
  return value;
}

double OnlMonParam::GetDoubleParam(const std::string par_name)
{
  istringstream iss(FindParamInDir(par_name));
  double value = 0;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  '" << par_name << "' is not double.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) cout << "  par_value = " << value << " as double." << endl;
  return value;
}

int OnlMonParam::GetIntParam(const std::string par_name)
{
  istringstream iss(FindParamInDir(par_name));
  int value = 0;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  '" << par_name << "' is not int.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) cout << "  par_value = " << value << " as int." << endl;
  return value;
}

bool OnlMonParam::GetBoolParam(const std::string par_name)
{
  istringstream iss(FindParamInDir(par_name));
  bool value = false;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  '" << par_name << "' is not bool.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) cout << "  par_value = " << value << " as bool." << endl;
  return value;
}

std::string OnlMonParam::FindParamInDir(const std::string par_name)
{
  char* path = gSystem->ExpandPathName(m_dir_base.c_str());
  string dir_name = path;
  delete path;
  dir_name += "/" + m_name;
  int run = m_run_id>0 ? m_run_id : recoConsts::instance()->get_IntFlag("RUNNUMBER");
  if (m_verb > 1) {
    cout << "OnlMonParam: par_name = " << par_name << " run = " << run << ".\n"
         << "  Directory = " << dir_name << endl;
  }

  string par_value = "";
  bool not_found = true;
  void* dirp = gSystem->OpenDirectory(dir_name.c_str());
  if (dirp) {
    const char* file_name_char;
    while (not_found && (file_name_char = gSystem->GetDirEntry(dirp))) {
      string file_name = file_name_char;
      int length = file_name.length();
      if (length >= 4 &&
          file_name.substr(length-4, 4) == ".tsv" &&
          FindParamInFile(dir_name+"/"+file_name, run, par_name, par_value) ) {
        not_found = false;
        break;
      }
    }
    gSystem->FreeDirectory(dirp);
  }
  if (not_found && m_do_assert) {
    cerr << "ERROR:  OnlMonParam cannot find '" << par_name << "' for run " << run << ".\n"
         << "Abort." << endl;
    exit(1);
  }
  return par_value;
}

bool OnlMonParam::FindParamInFile(const std::string file_name, const int run, const std::string par_name, std::string& par_value)
{
  if (m_verb > 1) {
    cout << "  File = " << file_name << endl;
  }
  bool do_found = false;
  ifstream ifs(file_name);
  string line;
  while (getline(ifs, line)) {
    if (line.length() == 0 || line[0] == '#') continue;
    if (m_verb > 2) {
      cout << "    Line = " << line << endl;
    }
    istringstream iss(line);
    string name, value;
    int run_b, run_e;
    if ((iss >> name >> value >> run_b >> run_e) &&
        par_name == name &&
        run_b <= run &&
        (run_e == 0 || run <= run_e) ) {
      par_value = value;
      do_found = true;
      break;
    }
  }
  ifs.close();
  return do_found;
}
