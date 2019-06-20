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

pthread_mutex_t mutex;

std::string OnlMonServer::m_out_dir      = "/data2/e1039/onlmon/plots";
std::string OnlMonServer::m_onl_mon_host = "localhost";
int         OnlMonServer::m_onl_mon_port = 9081;

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
  if (Verbosity() > 2) cout << "OnlMonServer::StartServer(): start." << endl;
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
  if (Verbosity() > 1) cout << "OnlMonServer::StartServer(): finish." << endl;
  return;
}

void* OnlMonServer::FuncServer(void* arg)
{
  const int MONIPORT=9081;
  const int NUMMONIPORT=5;
  OnlMonServer* se = (OnlMonServer*)arg;

  if (se->Verbosity() > 1) cout << "OnlMonServer::FuncServer(): start." << endl;

  int MoniPort = MONIPORT;
  //  int thread_arg[5];
  //pthread_mutex_lock(&mutex);

  //TH1 *h1 = new TH1F("serverhisto","serverhisto info histo",2,0,1);
  //se->registerHisto(h1);

  TServerSocket *ss = NULL;
  sleep(5);
  do
    {
      if (ss)
        {
          delete ss;
        }
      ss = new TServerSocket(MoniPort, kTRUE);
      // Accept a connection and return a full-duplex communication socket.
//      se->PortNumber(MoniPort);
      MoniPort++;
      if ((MoniPort - MONIPORT) >= NUMMONIPORT)
        {
          ostringstream msg;
          msg << "Too many Online Monitors running on this machine, bailing out" ;
	  cout << msg.str() << endl;
          exit(1);
        }
      if (!ss->IsValid())
        {
          printf("Ignore ROOT error about socket in use, I try another one\n");
        }
    }
  while (!ss->IsValid()); // from do{}while

  if (se->Verbosity() > 1) cout << "OnlMonServer::RemoveSockets():" << endl;
  // root keeps a list of sockets and tries to close them when quitting.
  // this interferes with my own threading and makes valgrind crash
  // The solution is to remove the TServerSocket *ss from roots list of
  // sockets. Then it will leave this socket alone.
  int isock = gROOT->GetListOfSockets()->IndexOf(ss);
  gROOT->GetListOfSockets()->RemoveAt(isock);

  sleep(5);
  //pthread_mutex_unlock(&mutex);

 again:
  if (se->Verbosity() > 1) cout << "OnlMonServer::WaitForConnection():" << endl;
  TSocket *s0 = ss->Accept();
  if (!s0)
    {
      cout << "Server socket " << MONIPORT
	   << " in use, either go to a different node or" << endl
	   << "change MONIPORT in server/PortNumber.h and recompile" << endl
	   << "server and client" << endl;
      exit(1);
    }
  // mutex protected since writing of histo
  // to outgoing buffer and updating by other thread do not
  // go well together
  if (se->Verbosity() > 2)
    {
      TInetAddress adr = s0->GetInetAddress();
      cout << "got connection from " << endl;
      adr.Print();
    }
  //  cout << "try locking mutex" << endl;
  //pthread_mutex_lock(&mutex);
  //cout << "got mutex" << endl;
  se->HandleConnection(s0);
  //cout << "try releasing mutex" << endl;
  //pthread_mutex_unlock(&mutex);
  //cout << "mutex released" << endl;
  delete s0;
  //cout << "closing socket" << endl;
  //s0->Close();
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
      } else if (msg_str == "Ack") {
        if (Verbosity() > 2) cout << "  Acknowledged." << endl;
        continue;
      } else if (msg_str.substr(0, 7) == "SUBSYS:") {
        string name_subsys = msg_str.substr(7);
        Fun4AllHistoManager* hm = getHistoManager(name_subsys);
        if (Verbosity() > 2) cout << "  SUBSYS: " << name_subsys << " " << hm->nHistos() << endl;
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
        sock->Send("Finished");
      } else {
        if (Verbosity() > 2) cout << "  Unexpected string message (" << msg_str << ").  Ignore it." << endl;
        delete mess;
        mess = 0;
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
