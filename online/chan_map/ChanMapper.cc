#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include "DbSvc.h"
#include "ChanMapper.h"
using namespace std;

ChanMapper::ChanMapper()
{
  m_dir_base = "/data2/analysis/kenichi/e1039/chan_map";
  m_label    = "base";
  m_map_id   = "";
  m_header   = "";
}

ChanMapper::~ChanMapper()
{
  ;
}

void ChanMapper::SetMapIDbyFile(const std::string map_id)
{
  m_range.ReadFromFile(RangeFileName().c_str());
  if (! m_range.Find(map_id)) {
    cout << "  !WARNING!  SetMapIDbyFile():  This map ID '" << map_id
         << "' is not included in the run-range table.  OK?" << endl;
  }
  m_map_id = map_id;
}

void ChanMapper::SetMapIDbyDB(const std::string map_id)
{
  m_range.ReadFromDB(RangeFileName().c_str());
  if (! m_range.Find(map_id)) {
    cout << "  !WARNING!  SetMapIDbyDB():  This map ID '" << map_id
         << "' is not included in the run-range table.  OK?" << endl;
  }
  m_map_id = map_id;
}

void ChanMapper::SetMapIDbyFile(const int run)
{
  m_range.ReadFromFile(RangeFileName().c_str());
  m_map_id = m_range.Find(run);
}

void ChanMapper::SetMapIDbyDB(const int run)
{
  m_range.ReadFromDB(SchemaName());
  m_map_id = m_range.Find(run);
}

void ChanMapper::ReadFromFile()
{
  ReadFromLocalFile(MapFileName());
}

void ChanMapper::WriteToFile()
{
  WriteToLocalFile(MapFileName());
}

void ChanMapper::ReadFromLocalFile(const string fn_tsv)
{
  cout << "  ChanMapper::ReadFromFile(): " << fn_tsv << "...";
  ifstream ifs(fn_tsv.c_str());
  if (! ifs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << fn_tsv << "'." << endl;
    exit(1);
  } 

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

void ChanMapper::WriteToLocalFile(const string fn_tsv)
{
  cout << "  ChanMapper::WriteToFile(): " << fn_tsv << "...";
  ofstream ofs(fn_tsv.c_str());
  if (! ofs) {
    cerr << "\n!!ERROR!!  Cannot open the map file '" << fn_tsv << "'." << endl;
    exit(1);
  }
  ofs << "#" << m_header << "\n";
  int nn = WriteFileCont(ofs);
  ofs.close();
  cout << " wrote " << nn << " entries." << endl;
}

void ChanMapper::ReadFromDB()
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

void ChanMapper::WriteToDB()
{
  cout << "ChanMapper::WriteToDB()\n";
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

void ChanMapper::WriteRangeToDB()
{
  m_range.WriteToDB(SchemaName());
}

void ChanMapper::Print(std::ostream& os)
{
  cout << "  virtual function called." << endl;
}

std::string ChanMapper::RangeFileName()
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_label << "/run_range.tsv";
  return oss.str();
}

std::string ChanMapper::MapFileName()
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_label << "/" << m_map_id << "/chan_map.tsv";
  return oss.str();
}

std::string ChanMapper::SchemaName()
{
  string ret = "user_e1039_chan_map_";
  ret += m_label;
  return ret;
}

std::string ChanMapper::MapTableName()
{
  string ret = "chan_map_";
  ret += m_map_id;
  return ret;
}

int ChanMapper::ReadFileCont(LineList& lines)
{
  cout << "  virtual function called." << endl;
  return 0;
}

int ChanMapper::WriteFileCont(std::ostream& os)
{
  cout << "  virtual function called." << endl;
  return 0;
}

void ChanMapper::ReadDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}

void ChanMapper::WriteDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}
