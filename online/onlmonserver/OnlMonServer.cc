#include <string>
#include <sstream>
#include <pthread.h>
#include <TMessage.h>
#include <TServerSocket.h>
#include <TSocket.h>
#include <TROOT.h>
#include <TH1.h>
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
  : Fun4AllServer(name), m_go_end(false)
{
  pthread_mutex_unlock(&mutex);
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
    sock.Send("Suicide");
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
  if (se->Verbosity() > 1) cout << "OnlMonServer::FuncServer(): start." << endl;

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
  if (se->Verbosity() > 1) cout << "OnlMonServer::RemoveSockets():" << endl;
  int isock = gROOT->GetListOfSockets()->IndexOf(ss);
  gROOT->GetListOfSockets()->RemoveAt(isock);
  sleep(5);

 again:
  if (se->Verbosity() > 1) cout << "OnlMonServer::WaitForConnection():" << endl;
  TSocket *s0 = ss->Accept();
  if (!s0) {
    cout << "Server socket " << port << " in use, either go to a different node or change the port and recompile server and client.  Abort." << endl;
    exit(1);
  }
  // mutex protected since writing of histo
  // to outgoing buffer and updating by other thread do not
  // go well together
  TInetAddress adr = s0->GetInetAddress();
  if (se->Verbosity() > 2) {
    cout << "got connection from\n  ";
    adr.Print();
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
  TMessage outgoing(kMESS_OBJECT);
  while (1) {
    if (Verbosity() > 2) cout << "OnlMonServer::HandleConnection(): while loop." << endl;
    sock->Recv(mess);
    if (! mess) {
      if (Verbosity() > 2) cout << "  Broken Connection, closing socket." << endl;
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
      } else if (msg_str == "Suicide") {
        cout << "OnlMonServer::HandleConnection():  Suicide." << endl;
        sock->Send("OK");
        m_go_end = true;
        break;
      } else if (msg_str == "Ping") {
        if (Verbosity() > 2) cout << "  Ping." << endl;
        sock->Send("Pong");
      } else if (msg_str.substr(0, 7) == "SUBSYS:") {
        string name_subsys = msg_str.substr(7);
        Fun4AllHistoManager* hm = getHistoManager(name_subsys);
        if (Verbosity() > 2) cout << "  SUBSYS: " << name_subsys << " " << hm->nHistos() << endl;

        pthread_mutex_lock(&mutex);
        for (unsigned int i = 0; i < hm->nHistos(); i++) {
          TH1 *histo = (TH1 *) hm->getHisto(i);
          if (! histo) continue;
          outgoing.Reset();
          outgoing.WriteObject(histo);
          sock->Send(outgoing);
          outgoing.Reset();
          sock->Recv(mess); // Just check a response.
          delete mess;
          mess = 0;
        }
        pthread_mutex_unlock(&mutex);

        sock->Send("Finished");
      } else {
        //if (Verbosity() > 2) 
        cout << "  Unexpected string message (" << msg_str << ").  Ignore it." << endl;
      }
    } else {
      cerr << "OnlMonServer::HandleConnection():  Unexpected message ("
           << mess->What() << ").  Ignore it." << endl;
      delete mess;
      mess = 0;
    }
  }
  
  sock->Close();
  return;
}
