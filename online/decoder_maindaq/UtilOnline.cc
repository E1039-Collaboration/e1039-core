#include <iostream>
#include <sstream>
#include <iomanip>
#include "UtilOnline.h"
using namespace std;

std::string UtilOnline::GetCodaFileDir()
{
  return "/data2/analysis/kenichi/e1039/codadata";
}

std::string UtilOnline::GetDstFileDir()
{
  return "/data2/analysis/kenichi/e1039/dst";
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

/// Convert a run number to the corresponding name of DST file.
std::string UtilOnline::RunNum2DstFile(const int run)
{
  ostringstream oss;
  oss << setfill('0') << "run_" << setw(6) << run << "_spin.root";
  return oss.str();
}
