#ifndef __H_OnlMonServer__H__
#define __H_OnlMonServer__H__
#include <string>
#include <fun4all/Fun4AllServer.h>
#include <pthread.h>
class TSocket;

class OnlMonServer : public Fun4AllServer
{
  static std::string m_out_dir;
  static std::string m_onl_mon_host;
  static int         m_onl_mon_port;

  bool m_go_end;

 public:
  static OnlMonServer *instance();
  virtual ~OnlMonServer();

  static void SetOutDir(const std::string dir)  { m_out_dir      = dir ; }
  static void SetHost  (const std::string host) { m_onl_mon_host = host; }
  static void SetPort  (const int port)         { m_onl_mon_port = port; }
  static std::string GetOutDir() { return m_out_dir; }
  static std::string GetHost  () { return m_onl_mon_host; }
  static int         GetPort  () { return m_onl_mon_port; }

  void StartServer();
  bool CloseExistingServer();
  static void* FuncServer(void* arg);
  void HandleConnection(TSocket* sock);
  void SetGoEnd(const bool val) { m_go_end = val; }
  bool GetGoEnd()        { return m_go_end; }

#ifndef __CINT__
  void GetMutex(pthread_mutex_t &lock) {lock = mutex;}
  void SetThreadId(pthread_t &id) {serverthreadid = id;}
#endif

 protected:
  OnlMonServer(const std::string &name = "OnlMonServer");

#ifndef __CINT__
  pthread_mutex_t mutex; //< Control the access to subsystem histograms.
  pthread_t serverthreadid;
#endif

};

#endif
