#include <string>
#include <sstream>
#include <pthread.h>
#include <TMessage.h>
#include <TServerSocket.h>
#include <TSocket.h>
#include <TROOT.h>
#include <TH1.h>
#include "OnlMonServer.h"
#include "OnlMonClient.h"
#include "OnlMonComm.h"
using namespace std;

OnlMonComm* OnlMonComm::m_inst = 0;

OnlMonComm* OnlMonComm::instance()
{
  if (! m_inst) m_inst = new OnlMonComm();
  return m_inst;
}

OnlMonComm::OnlMonComm()
  : m_sp_mode(SP_ALL)
  , m_sp_num(1)
  , m_sp_lo(0)
  , m_sp_hi(0)
  , m_sp_min(0)
  , m_sp_max(0)
  , m_sp_sel(true)
  , m_n_sp_sel_max(600)
{
  ;
}

OnlMonComm::~OnlMonComm()
{
  ;
}

void OnlMonComm::SetSpillRangeLow(const int sp)
{
  m_sp_lo = sp;
}

void OnlMonComm::SetSpillRangeHigh(const int sp)
{
  m_sp_hi = sp;
}

void OnlMonComm::SetSpillRange(const int sp_lo, const int sp_hi)
{
  m_sp_lo = sp_lo;
  m_sp_hi = sp_hi;
}

void OnlMonComm::GetSpillRange(int& sp_lo, int& sp_hi)
{
  sp_lo = m_sp_lo;
  sp_hi = m_sp_hi;
}

void OnlMonComm::AddSpill(const int id)
{
  if (find(m_list_sp.begin(), m_list_sp.end(), id) == m_list_sp.end()) m_list_sp.push_back(id);
}

void OnlMonComm::FindFullSpillRange(int& id_min, int& id_max)
{
  if (m_list_sp.size() == 0) {
    id_min = id_max = 0;
  } else {
    sort(m_list_sp.begin(), m_list_sp.end());
    id_min = m_list_sp[0];
    id_max = m_list_sp[m_list_sp.size()-1];
  }
}

int OnlMonComm::ReceiveFullSpillRange()
{
  TSocket* sock = ConnectServer();
  if (! sock) return 1;
  sock->Send("Spill");

  TMessage *mess = 0;
  sock->Recv(mess);
  int ret = 0;
  if (mess && mess->What() == kMESS_STRING) {
    char str[200];
    mess->ReadString(str, 200);
    istringstream iss(str);
    if (! (iss >> m_sp_min >> m_sp_max >> m_sp_sel)) {
      m_sp_min = m_sp_max = 0;
      m_sp_sel = false;
    }
    delete mess;
  } else {
    ret = 1;
  }

  sock->Close();
  delete sock;
  return ret;
}

void OnlMonComm::GetFullSpillRange(int& id_min, int& id_max)
{
  id_min = m_sp_min;
  id_max = m_sp_max;
}

TSocket* OnlMonComm::ConnectServer()
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
        if (mess && mess->What() == kMESS_STRING) {
          char str[200];
          mess->ReadString(str, 200);
          if (strcmp(str, "Pong") == 0) port_ok = true;
          delete mess;
        }
      }
    }
    if (port_ok) {
      cout << "OnlMonComm::ConnectServer(): " << host << ":" << port << endl;
      break;
    }
    delete sock;
    sock = 0;
    port++;
    if (port >= port0 + n_port) port -= n_port;
  }
  if (sock) OnlMonServer::SetPort(port);
  return sock;
}
