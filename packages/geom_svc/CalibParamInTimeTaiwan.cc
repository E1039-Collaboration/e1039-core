#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <TGraphErrors.h>
#include <db_svc/DbSvc.h>
#include "GeomSvc.h"
#include "CalibParamInTimeTaiwan.h"
using namespace std;

CalibParamInTimeTaiwan::CalibParamInTimeTaiwan() :
  CalibParamBase("intime_taiwan", "det\tele\tcenter\twidth")
{
  ;
}

int CalibParamInTimeTaiwan::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    short ele;
    double center, width;
    if (! (iss >> det >> ele >> center >> width)) continue;
    Add(det, ele, center, width);
    nn++;
  }
  return nn;
}

int CalibParamInTimeTaiwan::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->ele << "\t"
       << it->center << "\t" << it->width << "\n";
    nn++;
  }
  return nn;
}

void CalibParamInTimeTaiwan::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select det_name, det, ele, center, width from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    string det_name = stmt->GetString(0);
    short  det      = stmt->GetInt   (1);
    short  ele      = stmt->GetInt   (2);
    double center   = stmt->GetDouble(3);
    double width    = stmt->GetDouble(4);
    Add(det_name, det, ele, center, width);
  }
  delete stmt;
}

void CalibParamInTimeTaiwan::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {    "det_name",      "det",      "ele", "center",  "width" };
  const char* list_type[] = { "VARCHAR(32)", "SMALLINT", "SMALLINT", "DOUBLE", "DOUBLE" };
  const int   n_var       = 5;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(det_name, det, ele, center, width) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " ('" << it->det_name << "', " << it->det << ", " << it->ele << ", "
        << it->center << ", " << it->width << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  CalibParamInTimeTaiwan::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void CalibParamInTimeTaiwan::Add(
  const std::string det, const short ele, 
  const double center, const double width)
{
  GeomSvc* geom = GeomSvc::instance();
  string det_new = det;
  int    ele_new = ele;
  geom->toLocalDetectorName(det_new, ele_new);
  int det_id = geom->getDetectorID(det_new);
  Add(det, det_id, ele, center, width);

  if (ele_new != ele) {
    cout << "!WARNING!  CalibParamInTimeTaiwan::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
         << "  From det = " << det << ", ele = " << ele << "\n"
         << "  To   det = " << det_new << "(id = " << det_id << "), ele = " << ele_new << "\n"
         << "  The mapping result will be incorrect!!" << endl;
  }
}

void CalibParamInTimeTaiwan::Add(
  const std::string det_name, const short det_id, const short ele, 
  const double center, const double width)
{
  ParamItem item;
  item.det_name = det_name;
  item.det      = det_id;
  item.ele      = ele;
  item.center   = center;
  item.width    = width;
  m_list.push_back(item);
  m_map[DetEle_t(det_id, ele)] = CenterWidth_t(center, width);
}

bool CalibParamInTimeTaiwan::Find(const short det, const short ele, double& center, double& width)
{
  DetEle_t key(det, ele);
  if (m_map.find(key) != m_map.end()) {
    CenterWidth_t* val = &m_map[key];
    center = val->first;
    width  = val->second;
    return true;
  } else if (ele != 0) { // Try to find an entry common to all elements.
    return Find(det, 0, center, width);
  }
  center = width = 0;
  return false;
}

void CalibParamInTimeTaiwan::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t"
       << it->center << "\t" << it->width << "\n";
    n_ent++;
  }
  cout << "  n = " << n_ent << endl;
}
