#include <sstream>
#include <TSystem.h>
#include <TSQLServer.h>
#include <interface_main/SQEvent.h>
#include <phool/getClass.h>
#include <fun4all/Fun4AllServer.h>
#include <db_svc/DbSvc.h>
#include <UtilAna/UtilOnline.h>
#include "Fun4AllSpillDstOutputManager.h"
using namespace std;

Fun4AllSpillDstOutputManager::Fun4AllSpillDstOutputManager(const string &dir_base, const string &myname)
  : Fun4AllDstOutputManager(myname, "")
  , m_dir_base(dir_base)
  , m_sp_step(10)
  , m_run_id(0)
  , m_sp_id_f(0)
  , m_db(0)
  , m_name_table("")
{
  ;
}

Fun4AllSpillDstOutputManager::~Fun4AllSpillDstOutputManager()
{
  if (m_db) {
    if (m_run_id != 0) DstFinished(m_run_id, m_sp_id_f, m_sp_id_f + m_sp_step - 1);
    delete m_db;
  }
}

int Fun4AllSpillDstOutputManager::Write(PHCompositeNode *startNode)
{
  SQEvent* evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
  if (! evt) {
    cout << PHWHERE << "SQEvent not found.  Abort." << endl;
    exit(1);
  }
  int run_id  =  evt->get_run_id();
  int sp_id_f = (evt->get_spill_id() / m_sp_step) * m_sp_step;
  if (m_run_id != run_id || m_sp_id_f != sp_id_f) {
    m_run_id  = run_id;
    m_sp_id_f = sp_id_f;

    if (dstOut) { /// Write out and close the current DST file.
      PHNodeIterator nodeiter(Fun4AllServer::instance()->topNode());
      PHCompositeNode* run = dynamic_cast<PHCompositeNode*>(nodeiter.findFirst("PHCompositeNode", "RUN"));
      if (! run) {
        cout << PHWHERE << "RUN not found.  Abort." << endl;
        exit(1);
      }
      WriteNode(run); // dstOut is deleted at the beginning of this function.

      if (m_db) DstFinished(m_run_id, m_sp_id_f - m_sp_step, m_sp_id_f - 1);
    }

    /// Open a new DST file.
    ostringstream oss;
    oss << m_dir_base << "/run_" << setfill('0') << setw(6) << run_id;
    gSystem->mkdir(oss.str().c_str(), true);
    oss << "/run_" << setw(6) << run_id << "_spill_" << setw(9) << sp_id_f << "_spin.root";
    outfilename = oss.str();
    dstOut = new PHNodeIOManager(outfilename.c_str(), PHWrite);
    if (!dstOut->isFunctional()) {
      delete dstOut;
      dstOut = 0;
      cout << PHWHERE << "Could not open " << oss.str() << ".  Abort." << endl;
      exit(1);
    }
    dstOut->SetCompressionLevel(3);

    if (m_db) DstStarted(m_run_id, m_sp_id_f, m_sp_id_f + m_sp_step - 1, outfilename);
  }
  return Fun4AllDstOutputManager::Write(startNode);
}

void Fun4AllSpillDstOutputManager::EnableDB(const bool refresh_db, const std::string name_table)
{
  m_db = new DbSvc(DbSvc::DB1);
  m_db->UseSchema(UtilOnline::GetSchemaMainDaq(), true);

  m_name_table = name_table;

  if (refresh_db && m_db->HasTable(m_name_table)) m_db->DropTable(m_name_table);
  if (! m_db->HasTable(m_name_table)) {
    DbSvc::VarList list;
    list.Add("run_id"        , "INT", true);
    list.Add("spill_id_first", "INT", true);
    list.Add("spill_id_last" , "INT", true);
    list.Add("file_name"     , "VARCHAR(256)");
    list.Add("status"        , "INT");
    list.Add("utime_b"       , "INT");
    list.Add("utime_e"       , "INT");
    m_db->CreateTable(m_name_table, list);
  }
}

void Fun4AllSpillDstOutputManager::DstStarted(const int run, const int spill_f, const int spill_l, const std::string file_name, int utime)
{
  if (utime == 0) utime = time(0);

  ostringstream oss;
  oss << "insert into " << m_name_table
      << " values(" << run << ", " << spill_f << ", " << spill_l << ", '" << file_name << "', 1, " << utime << ", 0)"
      << " on duplicate key update file_name = '" << file_name << "', status = 1, utime_b = " << utime << ", utime_e = 0";
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunStarted()." << endl;
    return;
  }
}

void Fun4AllSpillDstOutputManager::DstFinished(const int run, const int spill_f, const int spill_l, int utime)
{
  if (utime == 0) utime = time(0);

  ostringstream oss;
  oss << "update " << m_name_table << " set status = 2, utime_e = " << utime
      << " where run_id = " << run << " and spill_id_first = " << spill_f << " and spill_id_last = " << spill_l;
  if (! m_db->Con()->Exec(oss.str().c_str())) {
    cerr << "!!ERROR!!  DecoStatusDb::RunFinished()." << endl;
    return;
  }
}
