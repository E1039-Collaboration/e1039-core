#include <string>
#include <sstream>
#include <pthread.h>
#include <TMessage.h>
#include <TServerSocket.h>
#include <TSocket.h>
#include <TROOT.h>
#include <TH1.h>
#include "OnlMonComm.h"
#include "OnlMonClient.h"
#include "OnlMonServer.h"

using namespace std;

/// ROOTTHREAD & SERVER are the switches for the thread implementation between ROOT & POSIX.
/// SERVER is being used in E1039 at present, where ROOTTHREAD has never been tried actually.
//#define ROOTTHREAD
#ifndef ROOTTHREAD
#define SERVER
#endif
#ifdef SERVER
int ServerThread = 0;
#endif
#ifdef ROOTTHREAD
static TThread *ServerThread = NULL;
#endif

//std::string OnlMonServer::m_out_dir    = "/data2/e1039/onlmon/plots";
std::string OnlMonServer::m_mon_host   = "localhost";
int         OnlMonServer::m_mon_port   = 9081;
int         OnlMonServer::m_mon_port_0 = 9081;
int         OnlMonServer::m_mon_n_port = 5;

OnlMonServer *OnlMonServer::instance()
{
  if (! __instance)
  {
    __instance = new OnlMonServer();
  }
  OnlMonServer *onlmonserver = dynamic_cast<OnlMonServer *> (__instance);
    return onlmonserver;
}

OnlMonServer::OnlMonServer(const std::string &name)
  : Fun4AllServer(name)
  , m_is_online(true)
  , m_go_end(false)
  , m_svr_ready(false)
{
  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);

  int ret = pthread_mutex_init(&mutex, &mutex_attr);
  if (ret != 0) {
    cout << "WARNING:  pthread_mutex_init() returned " << ret << "." << endl;
  }
  return;
}

OnlMonServer::~OnlMonServer()
{
  return;
}

void OnlMonServer::StartServer()
{
  cout << "OnlMonServer::StartServer():  Start." << endl;
  //  gBenchmark->Start("phnxmon");

  //GetMutex(mutex);
  //pthread_mutex_lock(&mutex);
#if defined(SERVER) || defined(ROOTTHREAD)

  pthread_t ThreadId = 0;
  if (!ServerThread)
    {
      if (Verbosity() > 2) cout << "  Creating server thread." << endl;
#ifdef SERVER
      ServerThread = pthread_create(&ThreadId, NULL, FuncServer, this);
      SetThreadId(ThreadId);
#endif
#ifdef ROOTTHREAD
      ServerThread = new TThread(server, (void *)0);
      ServerThread->Run();
#endif
    }

#endif
  //pthread_mutex_unlock(&mutex);
  //if (Verbosity() > 1) cout << "OnlMonServer::StartServer(): finish." << endl;
  cout << "  Started." << endl;
  return;
}

int OnlMonServer::End()
{
  m_go_end = true;
  int ret = Fun4AllServer::End();
  for (int ii = 0; ii < 30; ii++) { // Wait for one minute at max
    if (m_svr_ready) break;
    sleep(2);
  }
  return ret;

//#if defined(SERVER) || defined(ROOTTHREAD)
//  if (! serverthreadid) return;
//  SetGoEnd(true);
//
//#ifdef SERVER
//  pthread_join(serverthreadid, 0);
//  cout << "  Joined!" << endl;
//#endif
//#ifdef ROOTTHREAD
//  ServerThread->Join(server); // not supported
//#endif
//
//#endif
}

/// Close an existing server process if such exists.
/** It is possible that multiple decoding processes run in parallel on multiple runs.
 *  This function tries to keep the server of only the lastest process (which is expected to hanle the lastest run).
 *
 * Actually this function does not work properly, as found on 2019-07-05.  The existing onlmon server does suicides itself, but its port is kept open until the whole server process finishes.  Thus probably we need to use multiple ports, as done in the original PHENIX code.
 */
bool OnlMonServer::CloseExistingServer(const int port)
{
  TSocket sock(m_mon_host.c_str(), port);
  if (sock.IsValid()) {
    cout << "  Close the existing onlmon server at " << port << "." << endl;
    sock.Send(("Suicide:"+to_string(runnumber)).c_str());
    //TMessage *mess = 0;
    //sock.Recv(mess); // Just check a response.
    //delete mess;
    return true;
  } else {
    //cout << "No onlmon server exists." << endl;
    return false;
  }
}

void* OnlMonServer::FuncServer(void* arg)
{
  OnlMonServer* se = (OnlMonServer*)arg;
  if (se->Verbosity() >= 0) cout << "OnlMonServer::FuncServer(): start." << endl;

  sleep(5);
  TServerSocket* ss = 0;
  int port;
  for (port = m_mon_port_0; port < m_mon_port_0 + m_mon_n_port; port++) {
    if (se->CloseExistingServer(port)) continue; // Not try to use the port closed now
    ss = new TServerSocket(port, kTRUE);
    if (ss->IsValid()) break;
    delete ss;
    ss = 0;
  }
  if (! ss) {
    cout << "Too many online-monitor servers are running.  Start none." << endl;
    return 0;
  }
  m_mon_port = port;
  cout << "  Port = " << port << endl;

  // root keeps a list of sockets and tries to close them when quitting.
  // this interferes with my own threading and makes valgrind crash
  // The solution is to remove the TServerSocket *ss from roots list of
  // sockets. Then it will leave this socket alone.

  if (se->Verbosity() >= 0) cout << "OnlMonServer::RemoveSockets():" << endl;
  int isock = gROOT->GetListOfSockets()->IndexOf(ss);
  gROOT->GetListOfSockets()->RemoveAt(isock);
  sleep(5);
  se->SetServerReady(true);

 again:
  if (se->Verbosity() >= 0) cout << "OnlMonServer::WaitForConnection():" << endl;
  TSocket *s0 = ss->Accept();
  if (!s0) {
    cout << "Server socket " << port << " in use, either go to a different node or change the port and recompile server and client.  Abort." << endl;
    exit(1);
  }
  // mutex protected since writing of histo
  // to outgoing buffer and updating by other thread do not
  // go well together
  TInetAddress adr = s0->GetInetAddress();
  if (se->Verbosity() >= 0) {
    cout << "Connection from " << adr.GetHostName() << "/" << adr.GetHostAddress() << ":" << adr.GetPort() << endl;
  }
  UInt_t ip0 = adr.GetAddress();
  if ((ip0 >> 16) == (192 << 8) + 168 || ip0 == (127 << 24) + 1) {
    se->HandleConnection(s0);
  } else {
    cout << "OnlMonServer::FuncServer():  Ignore a connection from WAN.\n  ";
    adr.Print();
  }
  
  delete s0;
  //s0->Close();
  if (se->GetGoEnd()) {
    cout << "OnlMonServer::FuncServer():  End." << endl;
    return 0;
  }
  goto again;
}

void OnlMonServer::HandleConnection(TSocket* sock)
{
  /*
    int val;
    sock->GetOption(kSendBuffer, val);
    printf("sendbuffer size: %d\n", val);
    sock->GetOption(kRecvBuffer, val);
    printf("recvbuffer size: %d\n", val);
  */
  TMessage *mess = NULL;
  while (1) {
    if (Verbosity() > 2) cout << "OnlMonServer::HandleConnection(): while loop." << endl;
    sock->Recv(mess);
    if (! mess) {
      if (Verbosity() > 2) cout << "  Broken Connection, closing socket." << endl;
      break;
    }
    if (m_go_end) {
      if (Verbosity() > 2) cout << "  Already going to end, closing socket." << endl;
      break;
    }
    
    if (mess->What() == kMESS_STRING) {
      char msg_str_c[64];
      mess->ReadString(msg_str_c, 64);
      string msg_str = msg_str_c;
      delete mess;
      mess = 0;
      if (Verbosity() > 2) cout << "  Received message: " << msg_str << endl;
      
      if (msg_str == "Finished") {
        break;
      } else if (msg_str.substr(0, 8) == "Suicide:") {
        int run_id = stoi(msg_str.substr(8));
        cout << "OnlMonServer::HandleConnection():  Suicide " << run_id << "." << endl;
        if (runnumber < run_id) {
          sock->Send("OK");
          m_go_end = true;
        } else {
          sock->Send("NG");
        }
        break;
      } else if (msg_str == "Ping") {
        if (Verbosity() > 2) cout << "  Ping." << endl;
        sock->Send("Pong");
      } else if (msg_str == "Spill") {
        if (Verbosity() > 2) cout << "  Spill." << endl;
        int id_min, id_max;
        OnlMonComm* comm = OnlMonComm::instance();
        comm->FindFullSpillRange(id_min, id_max);
        ostringstream oss;
        oss << id_min << " " << id_max << " " << comm->GetSpillSelectability();
        sock->Send(oss.str().c_str());
      } else if (msg_str.substr(0, 7) == "SUBSYS:") {
        istringstream iss(msg_str.substr(7));
        string name_subsys;
        int sp_min, sp_max;
        iss >> name_subsys >> sp_min >> sp_max;
        cout << "  Subsystem " << name_subsys << endl;
        SubsysReco* sub = getSubsysReco(name_subsys);
        if (! sub) {
          cout << " ... Not available." << endl;
          sock->Send("NotReady");
        } else {
          OnlMonClient* cli = dynamic_cast<OnlMonClient*>(sub);
          cli->SendHist(sock, sp_min, sp_max);
        }
      } else {
        //if (Verbosity() > 2) 
        cout << "  Unexpected string message (" << msg_str << ").  Ignore it." << endl;
        break;
      }
    } else {
      cerr << "OnlMonServer::HandleConnection():  Unexpected message ("
           << mess->What() << ").  Ignore it." << endl;
      delete mess;
      mess = 0;
      break;
    }
  }
  
  sock->Close();
  return;
}
