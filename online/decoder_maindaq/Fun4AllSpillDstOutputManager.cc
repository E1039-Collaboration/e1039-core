#include <sstream>
#include <TSystem.h>
#include <interface_main/SQEvent.h>
#include <phool/getClass.h>
#include <fun4all/Fun4AllServer.h>
#include "Fun4AllSpillDstOutputManager.h"
using namespace std;

Fun4AllSpillDstOutputManager::Fun4AllSpillDstOutputManager(const string &dir_base, const string &myname)
  : Fun4AllDstOutputManager(myname, "")
  , m_dir_base(dir_base)
  , m_sp_step(10)
  , m_run_id(0)
  , m_sp_id(0)
{
  ;
}

Fun4AllSpillDstOutputManager::~Fun4AllSpillDstOutputManager()
{
  ;
}

int Fun4AllSpillDstOutputManager::Write(PHCompositeNode *startNode)
{
  SQEvent* evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
  if (! evt) {
    cout << PHWHERE << "SQEvent not found.  Abort." << endl;
    exit(1);
  }
  int run_id = evt->get_run_id();
  int sp_id  = (evt->get_spill_id() / m_sp_step) * m_sp_step;
  if (m_run_id != run_id || m_sp_id != sp_id) {
    m_run_id = run_id;
    m_sp_id  = sp_id;

    if (dstOut) { /// Write out and close the current DST file.
      PHNodeIterator nodeiter(Fun4AllServer::instance()->topNode());
      PHCompositeNode* run = dynamic_cast<PHCompositeNode*>(nodeiter.findFirst("PHCompositeNode", "RUN"));
      if (! run) {
        cout << PHWHERE << "RUN not found.  Abort." << endl;
        exit(1);
      }
      WriteNode(run); // dstOut is deleted at the beginning of this function.
    }

    /// Open a new DST file.
    ostringstream oss;
    oss << m_dir_base << "/run_" << setfill('0') << setw(6) << run_id;
    gSystem->mkdir(oss.str().c_str(), true);
    oss << "/run_" << setw(6) << run_id << "_spill_" << setw(9) << sp_id << "_spin.root";
    outfilename = oss.str();
    dstOut = new PHNodeIOManager(outfilename.c_str(), PHWrite);
    if (!dstOut->isFunctional()) {
      delete dstOut;
      dstOut = 0;
      cout << PHWHERE << "Could not open " << oss.str() << ".  Abort." << endl;
      exit(1);
    }
    dstOut->SetCompressionLevel(3);
  }
  return Fun4AllDstOutputManager::Write(startNode);
}
