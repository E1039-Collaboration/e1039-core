#include <fstream>
#include <sstream>
#include <TH1D.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/getClass.h>
#include <interface_main/SQEvent.h>
#include "FilterByTrigger.h"
using namespace std;

FilterByTrigger::FilterByTrigger()
  : m_trig_bits(0)
  , m_fn_out("")
  , m_h1_evt_cnt(0)
  , m_evt      (0)
{
  ;
}

FilterByTrigger::~FilterByTrigger()
{
  if (m_h1_evt_cnt) delete m_h1_evt_cnt;
}

int FilterByTrigger::Init(PHCompositeNode* topNode)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int FilterByTrigger::InitRun(PHCompositeNode* topNode)
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

int FilterByTrigger::process_event(PHCompositeNode* topNode)
{
  if (m_h1_evt_cnt) m_h1_evt_cnt->Fill(1);

  if ((m_evt->get_trigger() & m_trig_bits) == 0) return Fun4AllReturnCodes::ABORTEVENT;

  if (m_h1_evt_cnt) m_h1_evt_cnt->Fill(2);
  return Fun4AllReturnCodes::EVENT_OK;
}

int FilterByTrigger::End(PHCompositeNode* topNode)
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

void FilterByTrigger::SetFpgaBits(const bool fpga1, const bool fpga2, const bool fpga3, const bool fpga4, const bool fpga5)
{
  if (fpga1) m_trig_bits |= 0x1 << SQEvent::MATRIX1;
  if (fpga2) m_trig_bits |= 0x1 << SQEvent::MATRIX2;
  if (fpga3) m_trig_bits |= 0x1 << SQEvent::MATRIX3;
  if (fpga4) m_trig_bits |= 0x1 << SQEvent::MATRIX4;
  if (fpga5) m_trig_bits |= 0x1 << SQEvent::MATRIX5;
}

void FilterByTrigger::SetNimBits(const bool nim1, const bool nim2, const bool nim3, const bool nim4, const bool nim5)
{
  if (nim1) m_trig_bits |= 0x1 << SQEvent::NIM1;
  if (nim2) m_trig_bits |= 0x1 << SQEvent::NIM2;
  if (nim3) m_trig_bits |= 0x1 << SQEvent::NIM3;
  if (nim4) m_trig_bits |= 0x1 << SQEvent::NIM4;
  if (nim5) m_trig_bits |= 0x1 << SQEvent::NIM5;
}
