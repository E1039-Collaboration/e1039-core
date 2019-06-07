#ifndef __H_OnlMonServer__H__
#define __H_OnlMonServer__H__
#include <string>
#include <fun4all/Fun4AllServer.h>
#include <pthread.h>
class TSocket;

class OnlMonServer : public Fun4AllServer
{
  static std::string onl_mon_host;
  static int         onl_mon_port;

 public:
  static OnlMonServer *instance();
  virtual ~OnlMonServer();

  static void SetHost(const std::string host) { onl_mon_host = host; }
  static void SetPort(const int port)         { onl_mon_port = port; }
  static std::string GetHost() { return onl_mon_host; }
  static int         GetPort() { return onl_mon_port; }

  void StartServer();
  static void* FuncServer(void* arg);
  void HandleConnection(TSocket* sock);

#ifndef __CINT__
  void GetMutex(pthread_mutex_t &lock) {lock = mutex;}
  void SetThreadId(pthread_t &id) {serverthreadid = id;}
#endif

 protected:
  OnlMonServer(const std::string &name = "OnlMonServer");

#ifndef __CINT__
  pthread_mutex_t mutex;
  pthread_t serverthreadid;
#endif

};

#endif
