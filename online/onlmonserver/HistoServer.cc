#include "HistoServer.h"
#include "OnlMonServer.h"

#include <TClass.h>
#include <TH1.h>
#include <TMessage.h>
#include <TServerSocket.h>
#include <TSocket.h>
#include <TROOT.h>

#include <sstream>
#include <string>
#include <pthread.h>

using namespace std;
//#define ROOTTHREAD

#ifndef ROOTTHREAD
#define SERVER
#endif

#ifdef SERVER
static void *server(void *);
int ServerThread = 0;
#endif

#ifdef ROOTTHREAD
static void *server(void *);
static TThread *ServerThread = NULL;
#endif

pthread_mutex_t mutex;

std::string onl_mon_server = "localhost";
int         onl_mon_port = 9081;

static int MONIPORT=9081;
static int NUMMONIPORT=5;

int setup_server()
{
  cout << "OnlMonServer::setup(): start" << endl;
  //  gBenchmark->Start("phnxmon");
  OnlMonServer *se = OnlMonServer::instance();
  //se->GetMutex(mutex);
  //pthread_mutex_lock(&mutex);
#if defined(SERVER) || defined(ROOTTHREAD)

  pthread_t ThreadId = 0;
  if (!ServerThread)
    {
      cout << "creating server thread" << endl;
#ifdef SERVER

      ServerThread = pthread_create(&ThreadId, NULL, server, (void *)NULL);
      se->SetThreadId(ThreadId);
#endif
#ifdef ROOTTHREAD

      ServerThread = new TThread(server, (void *)0);
      ServerThread->Run();
#endif

    }
#endif
  //pthread_mutex_unlock(&mutex);
  cout << "OnlMonServer::setup(): end" << endl;
  return 0;
}

static void *server(void *arg)
{
  cout << "OnlMonServer::CreateThread(): start" << endl;
  OnlMonServer *se = OnlMonServer::instance();
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

  cout << "OnlMonServer::RemoveSockets():" << endl;
  // root keeps a list of sockets and tries to close them when quitting.
  // this interferes with my own threading and makes valgrind crash
  // The solution is to remove the TServerSocket *ss from roots list of
  // sockets. Then it will leave this socket alone.
  int isock = gROOT->GetListOfSockets()->IndexOf(ss);
  gROOT->GetListOfSockets()->RemoveAt(isock);
  sleep(5);
  //pthread_mutex_unlock(&mutex);

 again:
  cout << "OnlMonServer::WaitForConnection():" << endl;
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
  handleconnection(s0);
  //cout << "try releasing mutex" << endl;
  //pthread_mutex_unlock(&mutex);
  //cout << "mutex released" << endl;
  delete s0;
  //cout << "closing socket" << endl;
  //s0->Close();
  goto again;
}

void
handleconnection(void *arg)
{
  TSocket *s0 = (TSocket *) arg;

  OnlMonServer *se = OnlMonServer::instance();

  /*
    int val;
    s0->GetOption(kSendBuffer, val);
    printf("sendbuffer size: %d\n", val);
    s0->GetOption(kRecvBuffer, val);
    printf("recvbuffer size: %d\n", val);
  */
  TMessage *mess = NULL;
  TMessage outgoing(kMESS_OBJECT);
  while (1)
    {
      cout << "OnlMonServer::handleconnection(): while loop." << endl;
      if (se->Verbosity() > 2)
        {
          cout << "Waiting for message" << endl;
        }
      s0->Recv(mess);
      if (! mess)
        {
          //cout << "Broken Connection, closing socket" << endl;
          break;
        }
      if (mess->What() == kMESS_STRING)
        {

          char str[64];
          mess->ReadString(str, 64);
          string strpp = str; // to be unified with str...
          delete mess;
          mess = 0;
          if (se->Verbosity() > 2) cout << "received message" << str << endl;

          if (!strcmp(str, "Finished"))
            {
              break;
            }
          else if (!strcmp(str, "Ack"))
            {
              continue;
            }
          else if (!strcmp(str, "HistoList"))
            {
              if (se->Verbosity() > 2)
                {
                  cout << "number of histos: " << endl; //se->nHistos() << endl;
                }
//              for (unsigned int i = 0; i < se->nHistos(); i++)
              for (unsigned int i = 0; i < 1; i++)
                {
                  if (se->Verbosity() > 2)
                    {
                      cout << "HistoName: " << se->getHistoName(i) << endl;
                    }
                  s0->Send(se->getHistoName(i));
                  int nbytes = s0->Recv(mess);
                  delete mess;
                  mess = 0;
                  if (nbytes <= 0)
                    {
                      ostringstream msg;

                      msg << "Problem receiving message: return code: " << nbytes ;
		      cout << msg.str() << endl;

                    }
                }
              s0->Send("Finished");
            }
          else if (strpp.substr(0, 7) == "SUBSYS:")
            {
              string name_subsys = strpp.substr(7);
              Fun4AllHistoManager* hm = se->getHistoManager(name_subsys);
              cout << "  SUBSYS: " << name_subsys << " " << hm->nHistos() << endl;
              for (unsigned int i = 0; i < hm->nHistos(); i++)
                {
                  TH1 *histo = (TH1 *) hm->getHisto(i);
                  if (histo)
                    {
                      outgoing.Reset();
                      outgoing.WriteObject(histo);
                      s0->Send(outgoing);
                      outgoing.Reset();
                      s0->Recv(mess);
                      delete mess;
                      mess = 0;
                    }
                }
              s0->Send("Finished");
            }
          else if (!strcmp(str, "LIST"))
            {
              s0->Send("go");
              while (1)
                {
                  char str[200];
                  s0->Recv(mess);
		  if (!mess)
		    {
		      break;
		    }
                  if (mess->What() == kMESS_STRING)
                    {
                      mess->ReadString(str, 200);
                      delete mess;
                      mess = 0;
                      if (!strcmp(str, "alldone"))
                        {
                          break;
                        }
                    }
                  TH1 *histo = (TH1 *) se->getHisto(str);
                  if (histo)
                    {
                      outgoing.Reset();
                      outgoing.WriteObject(histo);
                      s0->Send(outgoing);
                      outgoing.Reset();
                    }
                  else
                    {
                      s0->Send("unknown");
                    }
                  //		  delete mess;
                }
              s0->Send("Finished");
            }
          else
            {
              TH1 *histo = (TH1 *) se->getHisto(str);
              if (histo)
                {
                  //		  const char *hisname = histo->GetName();
                  outgoing.Reset();
                  outgoing.WriteObject(histo);
                  s0->Send(outgoing);
                  outgoing.Reset();
                  s0->Recv(mess);
                  delete mess;
                  s0->Send("Finished");
                }
              else
                {
                  s0->Send("UnknownHisto");
                }
            }
        }
      else if (mess->What() == kMESS_OBJECT)
        {
          printf("got object of class: %s\n", mess->GetClass()->GetName());
          delete mess;
        }
      else
        {
          printf("*** Unexpected message ***\n");
          delete mess;
        }
    }

  // Close the socket.
  s0->Close();
  return ;
}

//void receive_hist_all()
//{
//  TSocket sock(onl_mon_server.c_str(), MONIPORT);
//  sock.Send("ALL");
//
//  TMessage *mess = NULL;
//  while (true) { // incoming hist
//    sock.Recv(mess);
//    if (!mess) {
//      break;
//    } else if (mess->What() == kMESS_STRING) {
//      char str[200];
//      mess->ReadString(str, 200);
//      delete mess;
//      mess = 0;
//      if (!strcmp(str, "Finished")) break;
//    } else if (mess->What() == kMESS_OBJECT) {
//      TClass* cla = mess->GetClass();
//      TH1*    obj = (TH1*)mess->ReadObject(cla);
//      cout << "Receive a class: " << cla->GetName() << " " << obj->GetName() << endl;
//      obj->Print();
//      delete mess;
//      mess = 0;
//      sock.Send("got it");
//    }
//  }
//  sock.Close();
//}


//int monitor_subsys(const char* name, std::vector<TH1*>& hist_list)
//{
//  ostringstream oss;
//  oss << "SUBSYS:" << name;
//
//  TSocket sock(onl_mon_server.c_str(), MONIPORT);
//  sock.Send(oss.str().c_str());
//
//  TMessage *mess = NULL;
//  while (true) { // incoming hist
//    sock.Recv(mess);
//    if (!mess) {
//      break;
//    } else if (mess->What() == kMESS_STRING) {
//      char str[200];
//      mess->ReadString(str, 200);
//      delete mess;
//      mess = 0;
//      if (!strcmp(str, "Finished")) break;
//    } else if (mess->What() == kMESS_OBJECT) {
//      TClass* cla = mess->GetClass();
//      TH1*    obj = (TH1*)mess->ReadObject(cla);
//      cout << "Receive a class: " << cla->GetName() << " " << obj->GetName() << endl;
//      hist_list.push_back( (TH1*)obj->Clone() );
//      delete mess;
//      mess = 0;
//      sock.Send("NEXT"); // Any text is ok for now
//    }
//  }
//  sock.Close();
//  return 0;
//}
