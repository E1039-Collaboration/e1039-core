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

CalibParamXT::CalibParamXT() :
  CalibParamBase("xt_curve", "det\tt\tx\tdx")
{
  ;
}

CalibParamXT::~CalibParamXT()
{
  for (Map_t::iterator it = m_map_t2x .begin(); it != m_map_t2x .end(); it++) delete it->second;
  for (Map_t::iterator it = m_map_t2dx.begin(); it != m_map_t2dx.end(); it++) delete it->second;
  for (Map_t::iterator it = m_map_x2t .begin(); it != m_map_x2t .end(); it++) delete it->second;
  for (Map_t::iterator it = m_map_x2dt.begin(); it != m_map_x2dt.end(); it++) delete it->second;
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
    if (! (iss >> det >> t >> x >> dx)) continue;
    dt = dx * t / x; // Temporary solution!
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
       << it->t << "\t" << it->x << "\t" << it->dx << "\n";
    nn++;
  }
  return nn;
}

void CalibParamXT::ReadDbTable(DbSvc& db)
{
  ostringstream oss;
  oss << "select det_name, det, t, x, dx from " << MapTableName();
  TSQLStatement* stmt = db.Process(oss.str());
  while (stmt->NextResultRow()) {
    string det_name = stmt->GetString(0);
    short  det      = stmt->GetInt   (1);
    double t        = stmt->GetDouble(2);
    double x        = stmt->GetDouble(3);
    double dx       = stmt->GetDouble(4);
    double dt = dx * t / x; // Temporary solution!
    Add(det_name, det, t, x, dt, dx);
  }
  delete stmt;
}

void CalibParamXT::WriteDbTable(DbSvc& db)
{
  string name_table = MapTableName();

  const char* list_var [] = {    "det_name",      "det",      "t",      "x",     "dx" };
  const char* list_type[] = { "VARCHAR(32)", "SMALLINT", "DOUBLE", "DOUBLE", "DOUBLE" };
  const int   n_var       = 5;
  db.CreateTable(name_table, n_var, list_var, list_type);

  ostringstream oss;
  oss << "insert into " << name_table << "(det_name, det, t, x, dx) values";
  for (List_t::iterator it = m_list.begin(); it != m_list.end(); it++) {
    oss << " ('" << it->det_name << "', " << it->det << ", "
        << it->t << ", " << it->x << ", " << it->dx << "),";
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
  TGraphErrors* gr_t2x;
  TGraphErrors* gr_t2dx;
  TGraphErrors* gr_x2t;
  TGraphErrors* gr_x2dt;
  if (m_map_t2x.find(det_id) == m_map_t2x.end()) {
    m_map_t2x [det_id] = gr_t2x  = new TGraphErrors();
    m_map_t2dx[det_id] = gr_t2dx = new TGraphErrors();
    m_map_x2t [det_id] = gr_x2t  = new TGraphErrors();
    m_map_x2dt[det_id] = gr_x2dt = new TGraphErrors();
  } else {
    gr_t2x  = m_map_t2x [det_id];
    gr_t2dx = m_map_t2dx[det_id];
    gr_x2t  = m_map_x2t [det_id];
    gr_x2dt = m_map_x2dt[det_id];
  }
  int n_pt = gr_t2x->GetN();
  gr_t2x ->SetPoint(n_pt, t, x);
  gr_t2dx->SetPoint(n_pt, t, dx);
  gr_t2x ->SetPointError(n_pt, 0, dx);
  gr_x2t ->SetPoint(n_pt, x, t);
  gr_x2dt->SetPoint(n_pt, x, dt);
  gr_x2t ->SetPointError(n_pt, 0, dt);
}

/// To be obsolete.  Use `FindT2X()`.
bool CalibParamXT::Find(const short det, TGraphErrors*& gr_t2x, TGraphErrors*& gr_t2dx)
{
  return FindT2X(det, gr_t2x, gr_t2dx);
}

bool CalibParamXT::FindT2X(const short det, TGraphErrors*& gr_t2x, TGraphErrors*& gr_t2dx)
{
  if (m_map_t2x.find(det) != m_map_t2x.end()) {
    gr_t2x  = m_map_t2x [det];
    gr_t2dx = m_map_t2dx[det];
    return true;
  }
  gr_t2x = gr_t2dx = 0;
  return false;
}

bool CalibParamXT::FindX2T(const short det, TGraphErrors*& gr_x2t, TGraphErrors*& gr_x2dt)
{
  if (m_map_x2t.find(det) != m_map_x2t.end()) {
    gr_x2t  = m_map_x2t [det];
    gr_x2dt = m_map_x2dt[det];
    return true;
  }
  gr_x2t = gr_x2dt = 0;
  return false;
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
