#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include "DbSvc.h"
#include "ChanMapperRange.h"
#include "ChanMapper.h"
using namespace std;

ChanMapper::ChanMapper()
{
  m_dir_base = "/data2/analysis/kenichi/e1039/chan_map";
  m_label    = "base";
  m_map_id   = "";
  m_header   = "";
}

std::string ChanMapper::RangeFileName()
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_label << "/run_range.tsv";
  return oss.str();
}

std::string ChanMapper::SchemaName()
{
  string ret = "user_e1039_chan_map_";
  ret += m_label;
  return ret;
}

std::string ChanMapper::TableName()
{
  string ret = "chan_map_";
  ret += m_map_id;
  return ret;
}

void ChanMapper::ReadFromFile(const int run)
{
  ostringstream oss;
  oss << m_dir_base << "/" << m_label << "/run_range.tsv";
  ChanMapperRange range;
  range.ReadFromFile(oss.str().c_str());

  m_map_id = range.Find(run);
  oss.str("");
  oss << m_dir_base << "/" << m_label << "/" << m_map_id << "/chan_map.tsv";
  ReadFromFile(oss.str());
}

void ChanMapper::ReadFromDB(const int run)
{
  ChanMapperRange range;
  range.ReadFromDB(SchemaName());
  m_map_id = range.Find(run);
  ReadFromDB();
}

void ChanMapper::ReadFromFile(const string fn_tsv)
{
  ;
}

void ChanMapper::WriteToFile(const string fn_tsv)
{
  ;
}

void ChanMapper::ReadFromDB()
{
  if (m_map_id.length() == 0) {
    cerr << "  ERROR:  The map ID is not set.  Abort." << endl;
    exit(1);
  }
  string name_schema = SchemaName();
  string name_table  =  TableName();
  cout <<   "  Schema = " << name_schema
       << "\n  Table  = " << name_table << "\n";

  DbSvc db(DbSvc::DB1);
  db.UseSchema(name_schema);
  db.AssureTable(name_table);
  ReadDbTable(db);
}

void ChanMapper::WriteToDB()
{
  cout << "ChanMapper::WriteToDB()\n";
  if (m_map_id.length() == 0) {
    cerr << "  ERROR:  The map ID is not set.  Abort." << endl;
    exit(1);
  }
  string name_schema = SchemaName();
  string name_table  =  TableName();
  cout <<   "  Schema = " << name_schema
       << "\n  Table  = " << name_table << "\n";

  DbSvc db(DbSvc::DB1);
  db.UseSchema(name_schema, true);
  db.DropTable(name_table);
  WriteDbTable(db);
  cout <<   "  ...done." << endl;
}

void ChanMapper::Print(std::ostream& os)
{
  cout << "  virtual function called." << endl;
}

void ChanMapper::ReadDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}

void ChanMapper::WriteDbTable(DbSvc& db)
{
  cout << "  virtual function called." << endl;
}

