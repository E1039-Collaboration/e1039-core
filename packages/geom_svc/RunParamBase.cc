#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSystem.h>
#include <db_svc/DbSvc.h>
#include "RunParamBase.h"
using namespace std;

RunParamBase::RunParamBase(const std::string type, const std::string label, const std::string header) :
  m_type(type), m_label(label), m_header(header), m_map_id("")
{
  m_dir_base = gSystem->Getenv("E1039_RESOURCE");
  if (m_dir_base.length() == 0) {
    m_dir_base = "/data2/e1039/resource";
    cout << "RunParamBase:  E1039_RESOURCE is empty.  Use '" << m_dir_base << "' as the data directory." << endl;
  }
}

void RunParamBase::SetMapIDbyFile(const std::string map_id)
{
  m_range.ReadFromFile(RangeFileName().c_str());
  if (! m_range.Find(map_id)) {
    cout << "  !WARNING!  SetMapIDbyFile():  This map ID '" << map_id
         << "' is not included in the run-range table.  OK?" << endl;
  }
  m_map_id = map_id;
}

void RunParamBase::SetMapIDbyDB(const std::string map_id)
{
  m_range.ReadFromDB(RangeFileName().c_str());
  if (! m_range.Find(map_id)) {
    cout << "  !WARNING!  SetMapIDbyDB():  This map ID '" << map_id
         << "' is not included in the run-range table.  OK?" << endl;
  }
  m_map_id = map_id;
}

void RunParamBase::SetMapIDbyFile(const int run)
{
  m_range.ReadFromFile(RangeFileName().c_str());
  m_map_id = m_range.Find(run);
}

void RunParamBase::SetMapIDbyDB(const int run)
{
  m_range.ReadFromDB(SchemaName());
  m_map_id = m_range.Find(run);
}

void RunParamBase::ReadFromFile()
{
  ReadFromLocalFile(MapFileName());
}

void RunParamBase::WriteToFile()
{
  WriteToLocalFile(MapFileName());
}

void RunParamBase::ReadFromLocalFile(const string fn_tsv)
{
  char* path = gSystem->ExpandPathName(fn_tsv.c_str());
  cout << "  RunParamBase::ReadFromFile(): " << path << "...";
  ifstream ifs(path);
  if (! ifs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << path << "'." << endl;
    exit(1);
  } 
  delete path;

  LineList lines;
  string buffer;
  while ( getline(ifs, buffer) ) {
    if (buffer[0] == '#') continue;
    lines.push_back(buffer);
  }
  ifs.close();
  int nn = ReadFileCont(lines);
  cout << " read " << nn << " entries." << endl;
}

void RunParamBase::WriteToLocalFile(const string fn_tsv)
{
  char* path = gSystem->ExpandPathName(fn_tsv.c_str());
  cout << "  RunParamBase::WriteToFile(): " << path << "...";
  ofstream ofs(path);
  if (! ofs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << path << "'." << endl;
    exit(1);
  }
  delete path;

  ofs << "#" << m_header << "\n";
  int nn = WriteFileCont(ofs);
  ofs.close();
  cout << " wrote " << nn << " entries." << endl;
}

void RunParamBase::ReadFromDB()
{
  if (m_map_id.length() == 0) {
    cerr << "  ERROR:  The map ID is not set.  Abort." << endl;
    exit(1);
  }
  string name_schema =   SchemaName();
  string name_table  = MapTableName();
  cout << "Read channel map from "
       << name_schema << "." << name_table << ".\n";
  //cout <<   "  Schema = " << name_schema
  //     << "\n  Table  = " << name_table << "\n";

  DbSvc db(DbSvc::DB1);
  db.UseSchema(name_schema);
  db.HasTable(name_table, true);
  ReadDbTable(db);
}

void RunParamBase::WriteToDB()
{
  cout << "RunParamBase::WriteToDB()\n";
  if (m_map_id.length() == 0) {
    cerr << "  ERROR:  The map ID is not set.  Abort." << endl;
    exit(1);
  }
  string name_schema =   SchemaName();
  string name_table  = MapTableName();
  cout <<   "  Schema = " << name_schema
       << "\n  Table  = " << name_table << "\n";

  DbSvc db(DbSvc::DB1);
  db.UseSchema(name_schema, true);
  db.DropTable(name_table);
  WriteDbTable(db);
  cout <<   "  ...done." << endl;
}

void RunParamBase::WriteRangeToDB()
{
  m_range.WriteToDB(SchemaName());
}

void RunParamBase::Print(std::ostream& os)
{
  cout << "  virtual function called." << endl;
}

std::string RunParamBase::RangeFileName()
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_type << "/" << m_label << "/run_range.tsv";
  return oss.str();
}

std::string RunParamBase::MapFileName()
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_type << "/" << m_label << "/" << m_map_id << "/param.tsv";
  return oss.str();
}

std::string RunParamBase::SchemaName()
{
  ostringstream oss;
  oss << "user_e1039_" << m_type << "_" << m_label;
  return oss.str();
}

std::string RunParamBase::MapTableName()
{
  ostringstream oss;
  oss << "param_" << m_map_id;
  return oss.str();
}

int RunParamBase::ReadFileCont(LineList& lines)
{
  cout << "  virtual function called." << endl;
  return 0;
}

int RunParamBase::WriteFileCont(std::ostream& os)
{
  cout << "  virtual function called." << endl;
  return 0;
}

void RunParamBase::ReadDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}

void RunParamBase::WriteDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}
