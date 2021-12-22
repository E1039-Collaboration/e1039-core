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
  , m_dir_base("/data2/e1039/onlmon/param")
  , m_run_id(0)
  , m_verb(2) // High default verbosity for now
  , m_do_assert(true)
{
  ;
}

OnlMonParam::OnlMonParam(const std::string name)
  : m_name(name)
  , m_dir_base("/data2/e1039/onlmon/param")
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

std::string OnlMonParam::GetCharParam(const std::string name)
{
  int run = m_run_id>0 ? m_run_id : recoConsts::instance()->get_IntFlag("RUNNUMBER");
  string dir_name = m_dir_base + "/" + m_name;
  if (m_verb > 0) {
    cout << "OnlMonParam: name = " << name << " run = " << run << ".\n"
         << "  Directory = " << dir_name << endl;
  }
  string value = "";
  bool not_found = true;
  void* dirp = gSystem->OpenDirectory(dir_name.c_str());
  if (dirp) {
    const char* name_char;
    while (not_found && (name_char = gSystem->GetDirEntry(dirp))) {
      if (m_verb > 0) {
        cout << "  File = " << name_char << endl;
      }
      string name = name_char;
      int length = name.length();
      if (length > 4 && name.substr(length-4, 4) == ".tsv") {
        ifstream ifs(name);
        string line;
        while (not_found && getline(ifs, line)) {
          if (m_verb > 1) {
            cout << "    Line = " << line << endl;
          }
          istringstream iss(line);
          string name0, value0;
          int run_b, run_e;
          if (! (iss >> name0 >> value0 >> run_b >> run_e)) {
            if (name == name0 && run_b <= run && (run_e == 0 || run <= run_e)) {
              if (m_verb > 1) {
                cout << "    Matched." << endl;
              }
              value = value0;
              not_found = false;
              break;
            }
          }
        }
        ifs.close();
      }
    }
    gSystem->FreeDirectory(dirp);
  }
  if (not_found && m_do_assert) {
    cerr << "ERROR:  OnlMonParam cannot find '" << name << "' for run " << run << ".\n"
         << "Abort." << endl;
    exit(1);
  }
  return value;
}

double OnlMonParam::GetDoubleParam(const std::string name)
{
  istringstream iss(GetCharParam(name));
  double value;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  OnlMonParam found '" << name << "' but not 'double'.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) {
    cout << "  " << value << " as double." << endl;
  }
  return value;
}

int OnlMonParam::GetIntParam(const std::string name)
{
  istringstream iss(GetCharParam(name));
  int value;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  OnlMonParam found '" << name << "' but not 'int'.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) {
    cout << "  " << value << " as int." << endl;
  }
  return value;
}

bool OnlMonParam::GetBoolParam(const std::string name)
{
  istringstream iss(GetCharParam(name));
  bool value;
  if (!(iss >> value) && m_do_assert) {
    cerr << "ERROR:  OnlMonParam found '" << name << "' but not 'bool'.\n"
         << "Abort." << endl;
    exit(1);
  }
  if (m_verb > 0) {
    cout << "  " << value << " as bool." << endl;
  }
  return value;
}

