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
#include "CalibParamInTimeV1495.h"
using namespace std;

CalibParamInTimeV1495::CalibParamInTimeV1495() :
  CalibParamBase("intime_v1495", "det\tele\tlvl\tcenter\twidth")
{
  ;
}

int CalibParamInTimeV1495::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    short ele, lvl;
    double center, width;
    if (! (iss >> det >> ele >> lvl >> center >> width)) continue;
    Add(det, ele, lvl, center, width);
    nn++;
  }
  return nn;
}

int CalibParamInTimeV1495::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->ele << "\t" << it->lvl << "\t"
       << it->center << "\t" << it->width << "\n";
    nn++;
  }
  return nn;
}

void CalibParamInTimeV1495::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select det_name, det, ele, lvl, center, width from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    string det_name = stmt->GetString(0);
    short  det      = stmt->GetInt   (1);
    short  ele      = stmt->GetInt   (2);
    short  lvl      = stmt->GetInt   (3);
    double center   = stmt->GetDouble(4);
    double width    = stmt->GetDouble(5);
    Add(det_name, det, ele, lvl, center, width);
  }
  delete stmt;
}

void CalibParamInTimeV1495::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {    "det_name",      "det",      "ele",      "lvl", "center",  "width" };
  const char* list_type[] = { "VARCHAR(32)", "SMALLINT", "SMALLINT", "SMALLINT", "DOUBLE", "DOUBLE" };
  const int   n_var       = 6;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(det_name, det, ele, lvl, center, width) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " ('" << it->det_name << "', " << it->det << ", " << it->ele << ", " << it->lvl << ", "
        << it->center << ", " << it->width << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  CalibParamInTimeV1495::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void CalibParamInTimeV1495::Add(
  const std::string det, const short ele, const short lvl, 
  const double center, const double width)
{
  GeomSvc* geom = GeomSvc::instance();
  string det_new = det;
  int    ele_new = ele;
  int    det_id;
  if      (det == "STOP"  ) { det_id = 1000; }
  else if (det == "L1PXtp") { det_id = 1001; }
  else if (det == "L1PXtn") { det_id = 1002; }
  else if (det == "L1PXbp") { det_id = 1003; }
  else if (det == "L1PXbn") { det_id = 1004; }
  else {
    geom->toLocalDetectorName(det_new, ele_new);
    det_id = geom->getDetectorID(det_new);
  }
  Add(det, det_id, ele, lvl, center, width);

  if (ele_new != ele) {
    cout << "!WARNING!  CalibParamInTimeV1495::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
         << "  From det = " << det << ", ele = " << ele << "\n"
         << "  To   det = " << det_new << "(id = " << det_id << "), ele = " << ele_new << "\n"
         << "  The mapping result will be incorrect!!" << endl;
  }
}

void CalibParamInTimeV1495::Add(
  const std::string det_name, const short det_id, const short ele, const short lvl, 
  const double center, const double width)
{
  ParamItem item;
  item.det_name = det_name;
  item.det      = det_id;
  item.ele      = ele;
  item.lvl      = lvl;
  item.center   = center;
  item.width    = width;
  m_list.push_back(item);
  m_map[DetEleLvl_t(det_id, ele, lvl)] = CenterWidth_t(center, width);
}

bool CalibParamInTimeV1495::Find(const short det, const short ele, const short lvl, double& center, double& width)
{
  DetEleLvl_t key(det, ele, lvl);
  if (m_map.find(key) != m_map.end()) {
    CenterWidth_t* val = &m_map[key];
    center = val->first;
    width  = val->second;
    return true;
  } else if (ele != 0) { // Try to find an entry common to all elements.
    return Find(det, 0, lvl, center, width);
  }
  center = width = 0;
  return false;
}

void CalibParamInTimeV1495::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t" << it->lvl << "\t"
       << it->center << "\t" << it->width << "\n";
    n_ent++;
  }
  cout << "  n = " << n_ent << endl;
}
