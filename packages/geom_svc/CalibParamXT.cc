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
#include "CalibParamXT.h"
using namespace std;

void CalibParamXT::Set::Add(const double t, const double x, const double dt, const double dx)
{
  int n_pt = t2x.GetN();
  t2x .SetPoint     (n_pt, t, x);
  t2dx.SetPoint     (n_pt, t, dx);
  t2dt.SetPoint     (n_pt, t, dt);
  t2x .SetPointError(n_pt, 0, dx);
  x2t .SetPoint     (n_pt, x, t);
  x2dt.SetPoint     (n_pt, x, dt);
  x2dx.SetPoint     (n_pt, x, dx);
  x2t .SetPointError(n_pt, 0, dt);
  if (n_pt == 0 || X0 > x) X0 = x;
  if (n_pt == 0 || X1 < x) X1 = x;
  if (n_pt == 0 || T0 < t) T0 = t;
  if (n_pt == 0 || T1 > t) T1 = t;
}

////////////////////////////////////////////////////////////////

CalibParamXT::CalibParamXT() :
  CalibParamBase("xt_curve", "det\tt\tx\tdt\tdx")
{
  ;
}

CalibParamXT::~CalibParamXT()
{
  ;
}

int CalibParamXT::ReadFileCont(LineList& lines)
{
  istringstream iss;
  int nn = 0;
  for (LineList::iterator it = lines.begin(); it != lines.end(); it++) {
    iss.clear(); // clear any error flags
    iss.str(*it);
    string det;
    double t, x, dt, dx;
    if (! (iss >> det >> t >> x >> dt >> dx)) continue;
    Add(det, t, x, dt, dx);
    nn++;
  }
  return nn;
}

int CalibParamXT::WriteFileCont(std::ostream& os)
{
  int nn = 0;
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    os << it->det_name << "\t"
       << it->t << "\t" << it->x << "\t" << it->dt << "\t" << it->dx << "\n";
    nn++;
  }
  return nn;
}

void CalibParamXT::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select det_name, det, t, x, dt, dx from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    string det_name = stmt->GetString(0);
    short  det      = stmt->GetInt   (1);
    double t        = stmt->GetDouble(2);
    double x        = stmt->GetDouble(3);
    double dt       = stmt->GetDouble(4);
    double dx       = stmt->GetDouble(5);
    Add(det_name, det, t, x, dt, dx);
  }
  delete stmt;
}

void CalibParamXT::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {    "det_name",      "det",      "t",      "x",     "dt",     "dx" };
  const char* list_type[] = { "VARCHAR(32)", "SMALLINT", "DOUBLE", "DOUBLE", "DOUBLE", "DOUBLE" };
  const int   n_var       = 6;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(det_name, det, t, x, dt, dx) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " ('" << it->det_name << "', " << it->det << ", "
        << it->t << ", " << it->x << ", " << it->dt << ", " << it->dx << "),";
  }
  string query = oss.str();
  query.erase(query.length()-1, 1); // Remove the last ',' char.
  if (! db.Con()->Exec(query.c_str())) {
    cerr << "!!ERROR!!  CalibParamXT::WriteToDB():  in insert.  Abort." << endl;
    exit(1);
  }
}

void CalibParamXT::Add(
  const std::string det,
  const double t, const double x, const double dt, const double dx)
{
  /// The X-T curve of the prop tubes is given per plane pair in the TSV file (at present), 
  /// namely P1H, P1V, P2V and P2H.  Since CalibParamXT needs one X-T curve per plane,
  /// two X-T curves are being made from one input (ex. P1H1f & P1H1b from P1H).
  /// In the future, the X-T curve is expected to be given per plane, like the drift chamber.
  if (det[0] == 'P' && det.length() == 3) {
    Add(det + "1f", t, x, dt, dx);
    Add(det + "1b", t, x, dt, dx);
    return;
  }

  const int ele = 1; // Any element is OK since the X-T curve is single per plane.
  GeomSvc* geom = GeomSvc::instance();
  string det_new = det;
  int    ele_new = ele;
  geom->toLocalDetectorName(det_new, ele_new);
  int det_id = geom->getDetectorID(det_new);
  Add(det, det_id, t, x, dt, dx);

  //if (ele_new != ele) {
  //  cout << "!WARNING!  CalibParamXT::Add():  The GeomSvc conversion changed element ID unexpectedly:\n"
  //       << "  From det = " << det << ", ele = " << ele << "\n"
  //       << "  To   det = " << det_new << " (id=" << det_id << "), ele = " << ele_new << "\n"
  //       << "  The mapping result will be incorrect!!" << endl;
  //}
}

void CalibParamXT::Add(
  const std::string det_name, const short det_id,
  const double t, const double x, const double dt, const double dx)
{
  ParamItem item;
  item.det_name = det_name;
  item.det      = det_id;
  item.t        = t;
  item.x        = x;
  item.dt       = dt;
  item.dx       = dx;
  m_list.push_back(item);

  m_map_sets[det_id].Add(t, x, dt, dx);
}

CalibParamXT::Set* CalibParamXT::GetParam(const short det)
{
  return m_map_sets.find(det) != m_map_sets.end()  ?  &m_map_sets[det]  :  0;
}

void CalibParamXT::Print(std::ostream& os)
{
  //int n_ent = 0;
  //for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
  //  os << it->det_name << "\t" << it->det << "\t" << it->ele << "\t"
  //     << it->roc << "\t" << it->board << "\t" << it->chan << "\n";
  //  n_ent++;
  //}
  //cout << "  n = " << n_ent << endl;
}
