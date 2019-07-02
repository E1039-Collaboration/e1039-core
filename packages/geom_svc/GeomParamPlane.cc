#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <TSQLServer.h>
#include <TSQLStatement.h>
#include <db_svc/DbSvc.h>
#include "GeomParamPlane.h"
using namespace std;

GeomParamPlane::GeomParamPlane() :
  GeomParamBase("plane", "to\tbe\tfilled")
{
  ;
}

int GeomParamPlane::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    Plane pl;
    if (! (iss >> pl.det_name >> pl.n_ele >> pl.cell_spacing >> pl.cell_width >> pl.angle_from_vert >> pl.xoffset >> pl.width >> pl.height >> pl.x0 >> pl.y0 >> pl.z0 >> pl.theta_x >> pl.theta_y >> pl.theta_z)) continue;
    Add(pl);
    nn++;
  }
  return nn;
}

int GeomParamPlane::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t" << it->n_ele << "\t" << it->cell_spacing << "\t" << it->cell_width << "\t" << it->angle_from_vert << "\t" << it->xoffset << "\t" << it->width << "\t" << it->height << "\t" << it->x0 << "\t" << it->y0 << "\t" << it->z0 << "\t" << it->theta_x << "\t" << it->theta_y << "\t" << it->theta_z << "\n";
    nn++;
  }
  return nn;
}

void GeomParamPlane::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select det_name, n_ele, cell_spacing, cell_width, angle_from_vert, xoffset, width, height, x0, y0, z0, theta_x, theta_y, theta_z  from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    Plane pl;
    pl.det_name        = stmt->GetString( 0);
    pl.n_ele           = stmt->GetInt   ( 1);
    pl.cell_spacing    = stmt->GetDouble( 2);
    pl.cell_width      = stmt->GetDouble( 3);
    pl.angle_from_vert = stmt->GetDouble( 4);
    pl.xoffset         = stmt->GetDouble( 5);
    pl.width           = stmt->GetDouble( 6);
    pl.height          = stmt->GetDouble( 7);
    pl.x0              = stmt->GetDouble( 8);
    pl.y0              = stmt->GetDouble( 9);
    pl.z0              = stmt->GetDouble(10);
    pl.theta_x         = stmt->GetDouble(11);
    pl.theta_y         = stmt->GetDouble(12);
    pl.theta_z         = stmt->GetDouble(13);
    Add(pl);
  }
  delete stmt;
}

void GeomParamPlane::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  DbSvc::VarList list;
  list.Add("det_name"       , "VARCHAR(16)", true);
  list.Add("n_ele"          , "INT");
  list.Add("cell_spacing"   , "DOUBLE");
  list.Add("cell_width"     , "DOUBLE");
  list.Add("angle_from_vert", "DOUBLE");
  list.Add("xoffset"        , "DOUBLE");
  list.Add("width"          , "DOUBLE");
  list.Add("height"         , "DOUBLE");
  list.Add("x0"             , "DOUBLE");
  list.Add("y0"             , "DOUBLE");
  list.Add("z0"             , "DOUBLE");
  list.Add("theta_x"        , "DOUBLE");
  list.Add("theta_y"        , "DOUBLE");
  list.Add("theta_z"        , "DOUBLE");
  db.CreateTable(name_table, list);

  ostringstream oss;
  oss << "insert into " << name_table << "(det_name, n_ele, cell_spacing, cell_width, angle_from_vert, xoffset, width, height, x0, y0, z0, theta_x, theta_y, theta_z) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " ('" << it->det_name << "', " << it->n_ele << ", " << it->cell_spacing << ", " << it->cell_width << ", " << it->angle_from_vert << ", " << it->xoffset << ", " << it->width << ", " << it->height << ", " << it->x0 << ", " << it->y0 << ", " << it->z0 << ", " << it->theta_x << ", " << it->theta_y << ", " << it->theta_z << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  GeomParamPlane::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void GeomParamPlane::Add(const Plane& plane)
{
  m_list.push_back(plane);
  m_map[plane.det_name] = plane;
}

bool GeomParamPlane::Find(const std::string det_name, Plane*& plane)
{
  if (m_map.find(det_name) != m_map.end()) {
    plane = &m_map[det_name];
    return true;
  }
  plane = 0;
  return false;
}

void GeomParamPlane::Print(std::ostream& os)
{
  int n_ent = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << "To be implemented.\n";
    n_ent++;
  }
  cout << "  n = " << n_ent << endl;
}
