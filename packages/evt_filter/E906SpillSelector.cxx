#include <fstream>
#include <sstream>
#include <TSystem.h>
#include <TH1D.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include <interface_main/SQEvent.h>
#include "E906SpillSelector.h"
using namespace std;

E906SpillSelector::E906SpillSelector(const std::string &name)
  : SubsysReco(name)
  , m_fn_list("$E1039_RESOURCE/spill/e906/R009/good_spill_fy2017.txt")
  , m_fn_out("")
  , m_h1_evt_cnt(0)
  , m_evt(0)
{
  ;
}

E906SpillSelector::~E906SpillSelector()
{
  if (m_h1_evt_cnt) delete m_h1_evt_cnt;
}

int E906SpillSelector::Init(PHCompositeNode* topNode)
{
  char* path = gSystem->ExpandPathName(m_fn_list.c_str());
  if (Verbosity() > 0) {
    cout << Name() << ":  Read '" << path << "'." << endl;
  }
  ifstream ifs(path);
  int sp;
  while (ifs >> sp) m_list_spill_ok.push_back(sp);
  ifs.close();
  if (Verbosity() > 0) {
    cout << "  N of good spills = " << m_list_spill_ok.size() << "." << endl;
  }
  if (m_list_spill_ok.size() == 0) {
    cout << Name() << ":  No good spill was found.  Abort." << endl;
    exit(1);
  }
  delete path;
  return Fun4AllReturnCodes::EVENT_OK;
}

int E906SpillSelector::InitRun(PHCompositeNode* topNode)
{
  m_evt = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!m_evt) return Fun4AllReturnCodes::ABORTEVENT;

  if (m_fn_out != "") {
    m_h1_evt_cnt = new TH1D("h1_evt_cnt", "", 2, 0.5, 2.5);
    m_h1_evt_cnt->GetXaxis()->SetBinLabel(1, "All");
    m_h1_evt_cnt->GetXaxis()->SetBinLabel(2, "Accepted");
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int E906SpillSelector::process_event(PHCompositeNode* topNode)
{
  m_h1_evt_cnt->Fill(1);
  int spill_id = m_evt->get_spill_id();
  if (find(m_list_spill_ok.begin(), m_list_spill_ok.end(), spill_id) == m_list_spill_ok.end()) return Fun4AllReturnCodes::ABORTEVENT;
  m_h1_evt_cnt->Fill(2);
  return Fun4AllReturnCodes::EVENT_OK;
}

int E906SpillSelector::End(PHCompositeNode* topNode)
{
  if (m_h1_evt_cnt) {
    ofstream ofs(m_fn_out.c_str());
    for (int ib = 1; ib <= m_h1_evt_cnt->GetNbinsX(); ib++) {
      ofs << m_h1_evt_cnt->GetXaxis()->GetBinLabel(ib) << "\t"
          << m_h1_evt_cnt->GetBinContent(ib) << "\n";
    }
    ofs.close();
  }
  return Fun4AllReturnCodes::EVENT_OK;
}
