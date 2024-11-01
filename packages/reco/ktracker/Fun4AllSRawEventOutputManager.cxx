#include <cstdlib>
#include <string>
#include <iostream>
#include <iomanip>
#include <TSystem.h>
#include <TFile.h>
#include <TTree.h>
#include <phool/phool.h>
#include <phool/getClass.h>
#include <phool/PHNode.h>
#include <phool/PHNodeIOManager.h>
#include <phool/PHNodeIterator.h>
#include <interface_main/SQEvent.h>
#include <interface_main/SQSpillMap.h>
#include <interface_main/SQHitVector.h>
#include <ktracker/SRawEvent.h>
//#include <ktracker/UtilSRawEvent.h>
#include "UtilSRawEvent.h"
#include "Fun4AllSRawEventOutputManager.h"
using namespace std;

Fun4AllSRawEventOutputManager::Fun4AllSRawEventOutputManager(const string &myname)
  : Fun4AllOutputManager(myname)
  , m_file_name("sraw.root")
  , m_tree_name("save")
  , m_branch_name("rawEvent")
  , m_run_id(0)
  , m_file(0)
  , m_tree(0)
  , m_sraw(0)
  , m_evt(0)
  , m_sp_map(0)
  , m_hit_vec(0)
  , m_trig_hit_vec(0)
{
  ;
}

Fun4AllSRawEventOutputManager::~Fun4AllSRawEventOutputManager()
{
  CloseFile();
  if (m_sraw) delete m_sraw;
}

int Fun4AllSRawEventOutputManager::Write(PHCompositeNode *startNode)
{
  if (! m_evt) {
    m_evt          = findNode::getClass<SQEvent    >(startNode, "SQEvent");
    m_sp_map       = findNode::getClass<SQSpillMap >(startNode, "SQSpillMap");
    m_hit_vec      = findNode::getClass<SQHitVector>(startNode, "SQHitVector");
    m_trig_hit_vec = findNode::getClass<SQHitVector>(startNode, "SQTriggerHitVector");
    if (!m_evt || !m_hit_vec || !m_trig_hit_vec) {
      cout << PHWHERE << "Cannot find the SQ data nodes.  Abort." << endl;
      exit(1);
    }
  }
  if (! m_file) OpenFile();

  //int run_id = m_evt->get_run_id();
  int sp_id  = m_evt->get_spill_id();
  SQSpill* sp = m_sp_map ? m_sp_map->get(sp_id) : 0;
  UtilSRawEvent::SetEvent     (m_sraw, m_evt);
  UtilSRawEvent::SetSpill     (m_sraw, sp);
  UtilSRawEvent::SetHit       (m_sraw, m_hit_vec);
  UtilSRawEvent::SetTriggerHit(m_sraw, m_trig_hit_vec);
  m_tree->Fill();
  return 0;
}

void Fun4AllSRawEventOutputManager::CloseFile()
{
  if (Verbosity() > 0) cout << "Fun4AllSRawEventOutputManager::CloseFile(): run " << m_run_id << endl;
  if (! m_file) return;
  m_file->Write();
  m_file->Close();
  delete m_file;
  m_file = 0;
}

void Fun4AllSRawEventOutputManager::OpenFile()
{
  if (Verbosity() > 0) cout << "Fun4AllSRawEventOutputManager::OpenFile(): run " << m_run_id << ", file " << m_file_name << endl;
  m_file = new TFile(m_file_name.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    cout << PHWHERE << "Could not open " << m_file_name << ".  Abort." << endl;
    exit(1);
  }
  if (!m_sraw) m_sraw = new SRawEvent();
  m_tree = new TTree(m_tree_name.c_str(), "");
  m_tree->Branch(m_branch_name.c_str(), &m_sraw);
}
