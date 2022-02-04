#include <iostream>
#include <sstream>
#include <iomanip>
#include <TSystem.h>
#include "UtilOnline.h"
using namespace std;

std::string UtilOnline::m_dir_end     = "/seaquest/e906daq/coda/data/END";
std::string UtilOnline::m_dir_coda    = "/localdata/codadata"; // could be "/data3/data/mainDAQ" or "/data2/e1039/codadata".
std::string UtilOnline::m_dir_dst     = "/data2/e1039/dst";
std::string UtilOnline::m_dir_eddst   = "/data2/e1039/online/evt_disp";
std::string UtilOnline::m_dir_onlmon  = "/data2/e1039/onlmon/plots";
std::string UtilOnline::m_sch_maindaq = "user_e1039_maindaq";

void UtilOnline::UseOutputLocationForDevel()
{
  m_dir_dst     = "/data2/e1039/dst-devel";
  m_dir_onlmon  = "/data2/e1039/onlmon/plots-devel";
  m_sch_maindaq = "user_e1039_maindaq_devel";
}

void UtilOnline::SetEndFileDir(const std::string dir)
{
  m_dir_end = gSystem->ExpandPathName(dir.c_str());
}

void UtilOnline::SetCodaFileDir(const std::string dir)
{
  m_dir_coda = gSystem->ExpandPathName(dir.c_str());
}

void UtilOnline::SetDstFileDir(const std::string dir)
{
  m_dir_dst = gSystem->ExpandPathName(dir.c_str());
}

void UtilOnline::SetEDDstFileDir(const std::string dir)
{
  m_dir_eddst = gSystem->ExpandPathName(dir.c_str());
}

void UtilOnline::SetOnlMonDir(const std::string dir)
{
  m_dir_onlmon = gSystem->ExpandPathName(dir.c_str());
}

void UtilOnline::SetSchemaMainDaq(const std::string sch)
{
  m_sch_maindaq = sch; 
}

/// Convert the name of a Coda file to its run number.
/**
 * The 1st argument can be a path, i.e. "/path/to/run_******_spin.dat".
 */
int UtilOnline::CodaFile2RunNum(const std::string name)
{
  int length = name.length();
  if (length < 19) return 0; // run_******_spin.dat
  return atoi(name.substr(length-15, 6).c_str());
}

/// Convert a run number to the corresponding name of Coda file.
std::string UtilOnline::RunNum2CodaFile(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "run_" << setw(6) << run << "_spin.dat";
  return oss.str();
}

/// Convert a run number to the corresponding name of END file.
std::string UtilOnline::RunNum2EndFile(const int run)
{
  ostringstream oss;
  oss << run << ".end";
  return oss.str();
}

/// Convert a run number to the corresponding name of DST file.
std::string UtilOnline::RunNum2DstFile(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "run_" << setw(6) << run << "_spin.root";
  return oss.str();
}

/// Convert a run number to the corresponding name of edDST file.
std::string UtilOnline::RunNum2EDDstFile(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "run_" << setw(6) << run << "_evt_disp.root";
  return oss.str();
}

std::string UtilOnline::GetCodaFilePath(const int run)
{
  return GetCodaFileDir() + "/" + RunNum2CodaFile(run);
}

std::string UtilOnline::GetEndFilePath(const int run)
{
  return GetEndFileDir() + "/" + RunNum2EndFile(run);
}

std::string UtilOnline::GetDstFilePath(const int run)
{
  return GetDstFileDir() + "/" + RunNum2DstFile(run);
}

std::string UtilOnline::GetEDDstFilePath(const int run)
{
  return GetEDDstFileDir() + "/" + RunNum2EDDstFile(run);
}
