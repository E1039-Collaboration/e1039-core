#include <iomanip>
#include <algorithm>
#include <TStyle.h>
#include <TSystem.h>
#include <TH1D.h>
#include <TSocket.h>
#include <TClass.h>
#include <TMessage.h>
#include <interface_main/SQRun.h>
#include <interface_main/SQEvent.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <decoder_maindaq/UtilOnline.h>
#include "OnlMonServer.h"
#include "OnlMonCanvas.h"
#include "OnlMonClient.h"
using namespace std;

std::vector<OnlMonClient*> OnlMonClient::m_list_us;
bool OnlMonClient::m_bl_clear_us = true;

OnlMonClient::OnlMonClient() :
  SubsysReco("OnlMonClient"), m_title("Client Title"), m_hm(0), m_n_can(1), m_h1_basic_info(0)
{
  memset(m_list_can, 0, sizeof(m_list_can));
  m_list_us.push_back(this);
}

OnlMonClient::~OnlMonClient()
{
  if (! m_hm) delete m_hm;
  ClearHistList();
  ClearCanvasList();
  m_list_us.erase( find(m_list_us.begin(), m_list_us.end(), this) );
}

OnlMonClient* OnlMonClient::Clone()
{
  cerr << "!!ERROR!!  OnlMonClient::Clone(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::Init(PHCompositeNode* topNode)
{
  return InitOnlMon(topNode);
}

int OnlMonClient::InitRun(PHCompositeNode* topNode)
{
  /// These settings will be applied to histograms created in InitRunOnlMon().
  /// Fine tunes should be necessary in the future.
  /// Note: the label/title size is a percent of the pad _height_, and
  ///       the offset is a relative scale to the default distance from axis.
  gStyle->SetLabelSize  (0.05, "X");
  gStyle->SetTitleSize  (0.05, "X");
  gStyle->SetLabelSize  (0.06, "YZ");
  gStyle->SetTitleSize  (0.06, "YZ");
  gStyle->SetTitleOffset(0.90, "XY");
  gStyle->SetTitleSize  (0.10, "");

  m_hm = new Fun4AllHistoManager(Name());
  OnlMonServer::instance()->registerHistoManager(m_hm);
  m_h1_basic_info = new TH1D("h1_basic_info", "", 10, 0.5, 10.5);
  m_hm->registerHisto(m_h1_basic_info);

  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  m_h1_basic_info->SetBinContent(BIN_RUN, run_header->get_run_id());

  return InitRunOnlMon(topNode);
}

int OnlMonClient::process_event(PHCompositeNode* topNode)
{
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!event) return Fun4AllReturnCodes::ABORTEVENT;

  pthread_mutex_t mutex;
  OnlMonServer::instance()->GetMutex(mutex);
  pthread_mutex_lock(&mutex);

  m_h1_basic_info->SetBinContent(BIN_SPILL, event->get_spill_id());
  m_h1_basic_info->SetBinContent(BIN_EVENT, event->get_event_id());
  m_h1_basic_info->AddBinContent(BIN_N_EVT, 1);

  int ret = ProcessEventOnlMon(topNode);
  pthread_mutex_unlock(&mutex);
  return ret;
}

int OnlMonClient::End(PHCompositeNode* topNode)
{
  int run_id;
  GetBasicInfo(&run_id);

  ClearCanvasList();
  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii] = new OnlMonCanvas(Name(), Title(), ii);
    m_list_can[ii]->SetBasicInfo(run_id);
    m_list_can[ii]->PreDraw(true);
  }

  int ret = DrawMonitor();
  if (ret != 0) {
    cerr << "WARNING: OnlMonClient::End().\n" << endl;
  }

  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii]->PostDraw(true);
  }

  ostringstream oss;
  oss << UtilOnline::GetOnlMonDir() << "/" << setfill('0') << setw(6) << run_id;
  gSystem->mkdir(oss.str().c_str(), true);
  oss << "/" << Name() << ".root";
  m_hm->dumpHistos(oss.str());

  return EndOnlMon(topNode);
}

void OnlMonClient::GetBasicInfo(int* run_id, int* spill_id, int* event_id, int* n_evt)
{
  if (run_id  ) *run_id   = (int)m_h1_basic_info->GetBinContent(BIN_RUN);
  if (spill_id) *spill_id = (int)m_h1_basic_info->GetBinContent(BIN_SPILL);
  if (event_id) *event_id = (int)m_h1_basic_info->GetBinContent(BIN_EVENT);
  if (n_evt   ) *n_evt    = (int)m_h1_basic_info->GetBinContent(BIN_N_EVT);
}

TH1* OnlMonClient::FindMonHist(const std::string name, const bool non_null)
{
  for (HistList_t::iterator it = m_list_h1.begin(); it != m_list_h1.end(); it++) {
    if (name == (*it)->GetName()) return *it;
  }
  if (non_null) {
    cerr << "!!ERROR!!  OnlMonClient::FindMonHist() cannot find '" << name << "'.  Abort." << endl;
    exit(1);
  }
  return 0;
}

TObject* OnlMonClient::FindMonObj(const std::string name, const bool non_null)
{
  for (ObjList_t::iterator it = m_list_obj.begin(); it != m_list_obj.end(); it++) {
    if (name == (*it)->GetName()) return *it;
  }
  if (non_null) {
    cerr << "!!ERROR!!  OnlMonClient::FindMonObj() cannot find '" << name << "'.  Abort." << endl;
    exit(1);
  }
  return 0;
}

int OnlMonClient::InitOnlMon(PHCompositeNode* topNode)
{
  cerr << "!!ERROR!!  OnlMonClient::InitOnlMon(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::InitRunOnlMon(PHCompositeNode* topNode)
{
  cerr << "!!ERROR!!  OnlMonClient::InitRunOnlMon(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::ProcessEventOnlMon(PHCompositeNode* topNode)
{
  cerr << "!!ERROR!!  OnlMonClient::ProcessEventOnlMon(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::EndOnlMon(PHCompositeNode* topNode)
{
  cerr << "!!ERROR!!  OnlMonClient::EndOnlMon(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::FindAllMonHist()
{
  cerr << "!!ERROR!!  OnlMonClient::FindAllMonHist(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::DrawMonitor()
{
  cerr << "!!ERROR!!  OnlMonClient::DrawMonitor(): virtual function called.  Abort." << endl;
  exit(1);
}

int OnlMonClient::StartMonitor()
{
  if (GetClearUsFlag()) {
    for (SelfList_t::iterator it = m_list_us.begin(); it != m_list_us.end(); it++) {
      (*it)->ClearCanvasList();
    }
  } else { // Clear only its own canvases
    ClearCanvasList();
  }
  if (ReceiveHist() != 0) {
    cout << "WARNING: Probably the online-monitor server is NOT running.\n";
    return 1;
  }

  m_h1_basic_info = (TH1*)FindMonObj("h1_basic_info");
  FindAllMonHist();

  int run_id, spill_id, event_id, n_evt;
  GetBasicInfo(&run_id, &spill_id, &event_id, &n_evt);
  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii] = new OnlMonCanvas(Name(), Title(), ii);
    m_list_can[ii]->SetBasicInfo(run_id, spill_id, event_id, n_evt);
    m_list_can[ii]->PreDraw();
  }

  int ret = DrawMonitor();

  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii]->PostDraw();
  }

  return ret;
}

void OnlMonClient::RegisterHist(TH1* h1)
{
  if (m_hm) m_hm->registerHisto(h1);
  else {
    cerr << "WARNING:  OnlMonClient::RegisterHist():  Cannot register hist (" << h1->GetName()
         << ").  You must call this function in InitRunOnlMon(), not InitOnlMon().  Do nothing." << endl;
  }
}

int OnlMonClient::ReceiveHist()
{
  string host = OnlMonServer::GetHost();
  int   port  = OnlMonServer::GetPort();
  int   port0 = OnlMonServer::GetPort0();
  int n_port  = OnlMonServer::GetNumPorts();

  TSocket* sock = 0;
  for (int ip = 0; ip < n_port; ip++) {
    bool port_ok = false;
    sock = new TSocket(host.c_str(), port);
    if (sock->IsValid()) {
      sock->Send("Ping");
      if (sock->Select(TSocket::kRead, 2000) > 0) { // 2000 msec
        TMessage* mess = 0;
        sock->Recv(mess);
        if (mess->What() == kMESS_STRING) {
          char str[200];
          mess->ReadString(str, 200);
          if (strcmp(str, "Pong") == 0) port_ok = true;
        }
        delete mess;
      }
    }
    if (port_ok) {
      cout << "OnlMonClient::ReceiveHist(): " << host << ":" << port << " " << Name() << endl;
      break;
    }
    delete sock;
    sock = 0;
    port++;
    if (port >= port0 + n_port) port -= n_port;
  }
  if (! sock) return 1;
  OnlMonServer::SetPort(port);
  
  string comm = "SUBSYS:";
  comm += Name();
  sock->Send(comm.c_str());

  ClearHistList();

  TMessage *mess = NULL;
  while (true) { // incoming hist
    sock->Recv(mess);
    if (!mess) {
      break;
    } else if (mess->What() == kMESS_STRING) {
      char str[200];
      mess->ReadString(str, 200);
      delete mess;
      mess = 0;
      if (!strcmp(str, "Finished")) break;
    } else if (mess->What() == kMESS_OBJECT) {
      TClass*  cla = mess->GetClass();
      TObject* obj = mess->ReadObject(cla);
      cout << "  Receive: " << cla->GetName() << " " << obj->GetName() << endl;
      m_list_obj.push_back( obj->Clone() ); // copy

      //TH1*    obj = (TH1*)mess->ReadObject(cla);
      //cout << "  Receive: " << cla->GetName() << " " << obj->GetName() << endl;
      //m_list_h1.push_back( (TH1*)obj->Clone() ); // copy
      delete mess;
      mess = 0;
      sock->Send("NEXT"); // Any text is ok for now
    }
  }
  sock->Close();
  delete sock;
  return 0;
}

void OnlMonClient::ClearHistList()
{
  for (HistList_t::iterator it = m_list_h1.begin(); it != m_list_h1.end(); it++) {
    delete *it;
  }
  m_list_h1.clear();

  for (ObjList_t::iterator it = m_list_obj.begin(); it != m_list_obj.end(); it++) {
    delete *it;
  }
  m_list_obj.clear();
}

OnlMonCanvas* OnlMonClient::GetCanvas(const int num) 
{
  if (num >= m_n_can) {
    cerr << "ERROR  OnlMonClient::GetCanvas():  Num out of range (" << num << " >= " << m_n_can << ").  Abort.";
    exit(1);
  }
  return m_list_can[num]; 
}

void OnlMonClient::ClearCanvasList()
{
  for (int ii = 0; ii < m_n_can; ii++) {
    if (m_list_can[ii]) {
      delete m_list_can[ii];
      m_list_can[ii] = 0;
    }
  }
}

