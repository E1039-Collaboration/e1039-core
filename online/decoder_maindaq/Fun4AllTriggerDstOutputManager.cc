#include <sstream>
//#include <TSystem.h>
//#include <TSQLServer.h>
#include <interface_main/SQEvent.h>
#include <phool/getClass.h>
#include <fun4all/Fun4AllServer.h>
//#include <db_svc/DbSvc.h>
//#include <UtilAna/UtilOnline.h>
#include "Fun4AllTriggerDstOutputManager.h"
using namespace std;

Fun4AllTriggerDstOutputManager::Fun4AllTriggerDstOutputManager(const string &myname, const string &filename)
  : Fun4AllDstOutputManager(myname, filename)
  , m_trig_mask(0)
{
  ;
}

Fun4AllTriggerDstOutputManager::~Fun4AllTriggerDstOutputManager()
{
  ;
}

void Fun4AllTriggerDstOutputManager::SetTriggerMask(const int fpga_mask, const int nim_mask)
{
  m_trig_mask = (fpga_mask << SQEvent::MATRIX1) | (nim_mask << SQEvent::NIM1);
}

void Fun4AllTriggerDstOutputManager::SetTriggerMask(const bool fpga1, const bool fpga2, const bool fpga3, const bool fpga4, const bool fpga5, const bool nim1, const bool nim2, const bool nim3, const bool nim4, const bool nim5)
{
  m_trig_mask = 0;
  if (fpga1) m_trig_mask |= (0x1 << SQEvent::MATRIX1);
  if (fpga2) m_trig_mask |= (0x1 << SQEvent::MATRIX2);
  if (fpga3) m_trig_mask |= (0x1 << SQEvent::MATRIX3);
  if (fpga4) m_trig_mask |= (0x1 << SQEvent::MATRIX4);
  if (fpga5) m_trig_mask |= (0x1 << SQEvent::MATRIX5);
  if (nim1 ) m_trig_mask |= (0x1 << SQEvent::NIM1);
  if (nim2 ) m_trig_mask |= (0x1 << SQEvent::NIM2);
  if (nim3 ) m_trig_mask |= (0x1 << SQEvent::NIM3);
  if (nim4 ) m_trig_mask |= (0x1 << SQEvent::NIM4);
  if (nim5 ) m_trig_mask |= (0x1 << SQEvent::NIM5);
}

int Fun4AllTriggerDstOutputManager::Write(PHCompositeNode *startNode)
{
  SQEvent* evt = findNode::getClass<SQEvent>(startNode, "SQEvent");
  if (! evt) {
    cout << PHWHERE << "SQEvent not found.  Abort." << endl;
    exit(1);
  }
  if (evt->get_trigger() & m_trig_mask) return Fun4AllDstOutputManager::Write(startNode);
  return 0;
}
