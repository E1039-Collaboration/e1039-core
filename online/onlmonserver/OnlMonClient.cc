#include <sstream>
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
#include <UtilAna/UtilOnline.h>
#include "OnlMonServer.h"
#include "OnlMonCanvas.h"
#include "OnlMonComm.h"
#include "OnlMonClient.h"
using namespace std;

std::vector<OnlMonClient*> OnlMonClient::m_list_us;
bool OnlMonClient::m_bl_clear_us = true;

OnlMonClient::OnlMonClient()
  : SubsysReco("OnlMonClient")
  , m_title("Client Title")
  , m_hm(0)
  , m_n_can(1)
  , m_h1_basic_id(0)
  , m_h1_basic_cnt(0)
  , m_spill_id_pre(-1)
  , m_make_sp_hist(true)
{
  memset(m_list_can, 0, sizeof(m_list_can));
  m_list_us.push_back(this);
}

OnlMonClient::~OnlMonClient()
{
  if (! m_hm) delete m_hm;
  ClearSpillHist();
  ClearHistList(m_list_h1);
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
  if (! OnlMonServer::instance()->GetOnline()) DisableSpillHist();
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
  m_h1_basic_id  = new TH1D("h1_basic_id" , "", 10, 0.5, 10.5);
  m_h1_basic_cnt = new TH1D("h1_basic_cnt", "", 10, 0.5, 10.5);
  m_hm->registerHisto(m_h1_basic_id);
  m_hm->registerHisto(m_h1_basic_cnt);

  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) return Fun4AllReturnCodes::ABORTEVENT;
  m_h1_basic_id->SetBinContent(BIN_RUN, run_header->get_run_id());

  return InitRunOnlMon(topNode);
}

int OnlMonClient::process_event(PHCompositeNode* topNode)
{
  SQEvent* event = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!event) return Fun4AllReturnCodes::ABORTEVENT;

  pthread_mutex_t* mutex = OnlMonServer::instance()->GetMutex();
  int ret_mutex = pthread_mutex_lock(mutex);
  if (ret_mutex != 0) cout << "WARNING:  mutex_lock returned " << ret_mutex << "." << endl;

  int sp_id = event->get_spill_id();
  if (sp_id != m_spill_id_pre) { // New spill
    OnlMonComm* comm = OnlMonComm::instance();
    if (m_spill_id_pre >= 0 && m_make_sp_hist) { // Not first spill
      if (comm->GetNumSpills() <= comm->GetMaxNumSelSpills()) {
        MakeSpillHist(m_spill_id_pre, sp_id);
      } else {
        cout << Name() << ": Disable spill-by-spill hists." << endl;
        MakeSpillHist(m_spill_id_pre);
        DisableSpillHist();
      }
    }
    m_spill_id_pre = sp_id;
    OnlMonComm::instance()->AddSpill(sp_id);
    m_h1_basic_cnt->AddBinContent(BIN_N_SP, 1);
    m_h1_basic_id ->SetBinContent(BIN_RUN  , event->get_run_id());
    m_h1_basic_id ->SetBinContent(BIN_SPILL, sp_id);
    m_h1_basic_id ->SetBinContent(BIN_EVENT, event->get_event_id());
    int sp_curr = m_h1_basic_id->GetBinContent(BIN_SPILL_MIN);
    if (sp_curr <= 0 || sp_curr > sp_id) m_h1_basic_id->SetBinContent(BIN_SPILL_MIN, sp_id);
    sp_curr = m_h1_basic_id->GetBinContent(BIN_SPILL_MAX);
    if (sp_curr < sp_id) m_h1_basic_id->SetBinContent(BIN_SPILL_MAX, sp_id);
  }
  m_h1_basic_cnt->AddBinContent(BIN_N_EVT, 1);

  int ret = ProcessEventOnlMon(topNode);
  ret_mutex = pthread_mutex_unlock(mutex);
  if (ret_mutex != 0) cout << "WARNING:  mutex_unlock returned " << ret_mutex << "." << endl;
  return ret;
}

int OnlMonClient::End(PHCompositeNode* topNode)
{
  if (! m_hm) return Fun4AllReturnCodes::EVENT_OK;

  if (m_spill_id_pre >= 0 && m_make_sp_hist) {
    MakeSpillHist(m_spill_id_pre);
    DisableSpillHist(); // Necessary here to merge all spill hists.
  }
  ClearHistList(m_list_h1);
  for (unsigned int ih = 0; ih < m_hm->nHistos(); ih++) {
    TH1* h1_org = (TH1*)m_hm->getHisto(ih);
    TH1* h1 = (TH1*)h1_org->Clone(h1_org->GetName());
    m_list_h1.push_back(h1);
  }

  ClearCanvasList();
  int ret = DrawCanvas(true);
  if (ret != 0) {
    cerr << "WARNING: OnlMonClient::End(): ret = " << ret << endl;
  }

  int run_id;
  GetBasicID(&run_id);

  ostringstream oss;
  oss << UtilOnline::GetOnlMonDir() << "/" << setfill('0') << setw(6) << run_id;
  gSystem->mkdir(oss.str().c_str(), true);
  gSystem->Chmod(oss.str().c_str(), 0775);
  oss << "/" << Name() << ".root";
  m_hm->dumpHistos(oss.str());
  gSystem->Chmod(oss.str().c_str(), 0664);

  return EndOnlMon(topNode);
}

void OnlMonClient::GetBasicID(int* run_id, int* spill_id, int* event_id, int* spill_id_min, int* spill_id_max)
{
  if (run_id      ) *run_id       = (int)m_h1_basic_id->GetBinContent(BIN_RUN);
  if (spill_id    ) *spill_id     = (int)m_h1_basic_id->GetBinContent(BIN_SPILL);
  if (event_id    ) *event_id     = (int)m_h1_basic_id->GetBinContent(BIN_EVENT);
  if (spill_id_min) *spill_id_min = (int)m_h1_basic_id->GetBinContent(BIN_SPILL_MIN);
  if (spill_id_max) *spill_id_max = (int)m_h1_basic_id->GetBinContent(BIN_SPILL_MAX);
}

void OnlMonClient::GetBasicCount(int* n_evt, int* n_sp)
{
  if (n_evt) *n_evt = (int)m_h1_basic_cnt->GetBinContent(BIN_N_EVT);
  if (n_sp ) *n_sp  = (int)m_h1_basic_cnt->GetBinContent(BIN_N_SP );
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
  int ret = ReceiveHist();
  if (ret == 1) {
    cout << "WARNING: The OnlMon server is NOT running." << endl;
    return 1;
  } else if (ret == 2) {
    cout << "WARNING: The OnlMon server is NOT ready." << endl;
    return 1;
  } else if (ret != 0) {
    cout << "WARNING: Unknown error in connecting to the OnlMon server." << endl;
    return 1;
  }

  return DrawCanvas();
}

void OnlMonClient::RegisterHist(TH1* h1, const HistMode_t mode)
{
  if (m_hm) {
    m_hm->registerHisto(h1);
    m_hist_mode[h1->GetName()] = mode;
  } else {
    cerr << "WARNING:  OnlMonClient::RegisterHist():  Cannot register hist (" << h1->GetName()
         << ").  You must call this function in InitRunOnlMon(), not InitOnlMon().  Do nothing." << endl;
  }
}

int OnlMonClient::SendHist(TSocket* sock, int sp_min, int sp_max)
{
  if (! m_hm) {
    //if (Verbosity() > 2) cout << "  HM not ready." << endl;
    sock->Send("NotReady");
    return 1; // Not ready
  }

  pthread_mutex_t* mutex = OnlMonServer::instance()->GetMutex();
  int ret_mutex = pthread_mutex_lock(mutex);
  if (ret_mutex != 0) cout << "WARNING:  mutex_lock returned " << ret_mutex << "." << endl;

  HistList_t list_h1;
  if (m_make_sp_hist) {
    if (sp_min > 0 && sp_max == 0) { // "sp_min" means N of spills
      int min0, max0;
      OnlMonComm::instance()->FindFullSpillRange(min0, max0);
      sp_max = max0;
      sp_min = max0 - sp_min + 1;
    }
    MakeMergedHist(list_h1, sp_min, sp_max);
  } else {
    for (unsigned int ih = 0; ih < m_hm->nHistos(); ih++) {
      TH1* h1_org = (TH1*)m_hm->getHisto(ih);
      TH1* h1 = (TH1*)h1_org->Clone(h1_org->GetName());
      list_h1.push_back(h1);
    }
  }

  ret_mutex = pthread_mutex_unlock(mutex);
  if (ret_mutex != 0) cout << "WARNING:  mutex_unlock returned " << ret_mutex << "." << endl;

  TMessage outgoing(kMESS_OBJECT);
  //if (Verbosity() > 2) cout << "  SUBSYS: " << name_subsys << " " << hm->nHistos() << endl;
  for (HistList_t::iterator it = list_h1.begin(); it != list_h1.end(); it++) {
    TH1* h1 = *it;
    outgoing.Reset();
    outgoing.WriteObject(h1);
    sock->Send(outgoing);
    outgoing.Reset();
    TMessage* mess = 0;
    sock->Recv(mess); // Just check a response.
    delete mess;
  }
  ClearHistList(list_h1);

  sock->Send("Finished");
  return 0;
}

/// Clear the contents of all spill-by-spill histograms.
/**
 * This function deletes "N_spills * N_types" of histograms per client.
 * But the memory usage usually does not go down promptly.
 * It should be fine (i.e. not memory leak) as the memory will be released later when needed by other processes.
 * Such delay could happen when a huge number of small memory blocks are deleted, as done in this function.
 */
void OnlMonClient::ClearSpillHist()
{
  int n_type = 0;
  int n_hist = 0;
  for (Name2SpillHistMap_t::iterator it1 = m_map_hist_sp.begin(); it1 != m_map_hist_sp.end(); it1++) {
    SpillHistMap_t* map_hist = &it1->second;
    for (SpillHistMap_t::iterator it2 = map_hist->begin(); it2 != map_hist->end(); it2++) {
      delete it2->second;
      n_hist++;
    }
    n_type++;
  }
  m_map_hist_sp.clear();
  cout << Name() << ": Cleared " << n_hist << " hists for " << n_type << " types." << endl;
}

/**
 * This function makes a set of histograms for the given spill ID, "spill_id".
 * The set is copied from the histograms managed by "m_hm" into "m_map_hist_sp".
 * When "spill_id_new" is given, a new entry for that spill is added to "m_map_hist_sp",
 * which is just a set of pointers (not cloned objects) to the histograms managed by "m_hm".
 */
void OnlMonClient::MakeSpillHist(const int spill_id, const int spill_id_new)
{
  bool add_dir = TH1::AddDirectoryStatus();
  TH1::AddDirectory(false); // Significantly speeds up deleting hists in dtor.
  for (unsigned int ih = 0; ih < m_hm->nHistos(); ih++) {
    TH1* h1 = (TH1*)m_hm->getHisto(ih);
    string name = h1->GetName();
    ostringstream oss;
    oss << name << "_sp" << spill_id;
    m_map_hist_sp[name][spill_id] = (TH1*)h1->Clone(oss.str().c_str());
    h1->Reset("M");
    if (spill_id_new > 0) m_map_hist_sp[name][spill_id_new] = h1;
  }
  TH1::AddDirectory(add_dir);
}

/**
 * This function overwrites the contents of "m_hist_h1".
 * Thus "m_hist_h1" must be empty when this function is called.
 */
void OnlMonClient::DisableSpillHist()
{
  if (! m_make_sp_hist) return;
  if (m_map_hist_sp.size() > 0) { // Merge existing spill hists
    HistList_t list_h1;
    MakeMergedHist(list_h1);
    for (unsigned int ih = 0; ih < m_hm->nHistos(); ih++) {
      TH1* h1 = (TH1*)m_hm->getHisto(ih);
      h1->Reset("M");
      h1->Add(list_h1.at(ih));
    }
    ClearHistList(list_h1);
    ClearSpillHist();
  }
  m_make_sp_hist = false;
  OnlMonComm::instance()->SetSpillSelectability(false);
}

void OnlMonClient::MakeMergedHist(HistList_t& list_h1, const int sp_min, const int sp_max)
{
  bool add_dir = TH1::AddDirectoryStatus();
  TH1::AddDirectory(false);
  ClearHistList(list_h1);
  for (unsigned int ih = 0; ih < m_hm->nHistos(); ih++) {
    TH1* h1_org = (TH1*)m_hm->getHisto(ih);
    string name = h1_org->GetName();
    TH1* h1 = (TH1*)h1_org->Clone(name.c_str());
    h1->Reset("M");
    SpillHistMap_t* map_hist = &m_map_hist_sp[name];
    for (SpillHistMap_t::iterator it = map_hist->begin(); it != map_hist->end(); it++) {
      int sp_id = it->first;
      if (sp_min > 0 && sp_id < sp_min) continue;
      if (sp_max > 0 && sp_id > sp_max) continue;
      if (name == "h1_basic_id") {
        h1->SetBinContent(BIN_RUN  , TMath::Max(h1->GetBinContent(BIN_RUN  ), it->second->GetBinContent(BIN_RUN  )));
        h1->SetBinContent(BIN_SPILL, TMath::Max(h1->GetBinContent(BIN_SPILL), it->second->GetBinContent(BIN_SPILL)));
        h1->SetBinContent(BIN_EVENT, TMath::Max(h1->GetBinContent(BIN_EVENT), it->second->GetBinContent(BIN_EVENT)));
        int sp_curr = h1->GetBinContent(BIN_SPILL_MIN);
        if (sp_curr <= 0 || sp_curr > sp_id) h1->SetBinContent(BIN_SPILL_MIN, sp_id);
        sp_curr = h1->GetBinContent(BIN_SPILL_MAX);
        if (sp_curr < sp_id) h1->SetBinContent(BIN_SPILL_MAX, sp_id);
      } else if (m_hist_mode[name] == MODE_UPDATE) {
        if (it == --map_hist->end()) h1->Add(it->second); // Take the last one
      } else { // MODE_ADD
        h1->Add(it->second);
      }
    }
    list_h1.push_back(h1);
  }
  TH1::AddDirectory(add_dir);
}

int OnlMonClient::ReceiveHist()
{
  TSocket* sock = OnlMonComm::instance()->ConnectServer();
  if (! sock) return 1;

  int sp_lo, sp_hi;
  switch (OnlMonComm::instance()->GetSpillMode()) {
  case OnlMonComm::SP_ALL:
    sp_lo = sp_hi = 0;
    break;
  case OnlMonComm::SP_LAST:
    sp_lo = OnlMonComm::instance()->GetSpillNum();
    sp_hi = 0;
    break;
  case OnlMonComm::SP_RANGE:
    OnlMonComm::instance()->GetSpillRange(sp_lo, sp_hi);
    break;
  }

  ostringstream oss;
  oss << "SUBSYS:" << Name() << " " << sp_lo << " " << sp_hi;
  sock->Send(oss.str().c_str());

  ClearHistList(m_list_h1);

  int ret = 0;
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
      if      (strcmp(str, "Finished") == 0) break;
      else if (strcmp(str, "NotReady") == 0) {
        ret = 2;
        break;
      }
    } else if (mess->What() == kMESS_OBJECT) {
      TClass*  cla = mess->GetClass();
      TObject* obj = mess->ReadObject(cla);
      cout << "  Receive: " << cla->GetName() << " " << obj->GetName() << endl;
      m_list_h1.push_back( (TH1*)obj->Clone() ); // copy
      delete mess;
      mess = 0;
      sock->Send("NEXT"); // Any text is ok for now
    }
  }
  sock->Close();
  delete sock;
  return ret;
}

void OnlMonClient::ClearHistList(HistList_t& list_h1)
{
  for (HistList_t::iterator it = list_h1.begin(); it != list_h1.end(); it++) {
    delete *it;
  }
  list_h1.clear();
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

int OnlMonClient::DrawCanvas(const bool at_end)
{
  m_h1_basic_id  = FindMonHist("h1_basic_id");
  m_h1_basic_cnt = FindMonHist("h1_basic_cnt");
  int ret = FindAllMonHist();
  if (m_h1_basic_id == 0 || m_h1_basic_cnt == 0 || ret != 0) {
    cout << "WARNING: Cannot find OnlMon histogram(s)." << endl;
    return 2;
  }

  int run_id, spill_id, event_id, spill_id_min, spill_id_max;
  GetBasicID(&run_id, &spill_id, &event_id, &spill_id_min, &spill_id_max);
  int n_evt, n_sp;
  GetBasicCount(&n_evt, &n_sp);
  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii] = new OnlMonCanvas(Name(), Title(), ii);
    m_list_can[ii]->SetBasicID(run_id, spill_id, event_id, spill_id_min, spill_id_max);
    m_list_can[ii]->SetBasicCount(n_evt, n_sp);
    m_list_can[ii]->PreDraw(at_end);
  }

  ret = DrawMonitor();

  for (int ii = 0; ii < m_n_can; ii++) {
    m_list_can[ii]->PostDraw(at_end);
  }

  return ret;
}
