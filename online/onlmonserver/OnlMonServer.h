#ifndef __H_OnlMonServer__H__
#define __H_OnlMonServer__H__
#include <map>
#include <string>
#include <fun4all/Fun4AllServer.h>
#include <pthread.h>
class TSocket;
class OnlMonClient;

class OnlMonServer : public Fun4AllServer
{
  //typedef std::vector<int> SpillList_t;
  //SpillList_t m_list_sp;

  //typedef std::map<std::string, OnlMonClient*> ClientMap_t;
  //ClientMap_t m_map_cli;

  //static std::string m_out_dir;
  static std::string m_mon_host;
  static int         m_mon_port; //< The port being used
  static int         m_mon_port_0; //< The 1st number of available ports
  static int         m_mon_n_port; //< The number of available ports

  bool m_go_end;

 public:
  static OnlMonServer *instance();
  virtual ~OnlMonServer();

  //void AddClient(OnlMonClient* cli);

  //static void SetOutDir  (const std::string dir)  { m_out_dir    = dir ; }
  static void SetHost    (const std::string host) { m_mon_host   = host; }
  static void SetPort    (const int port)         { m_mon_port   = port; }
  static void SetPort0   (const int port)         { m_mon_port_0 = port; }
  static void SetNumPorts(const int num )         { m_mon_n_port = num ; }
  //static std::string GetOutDir  () { return m_out_dir   ; }
  static std::string GetHost    () { return m_mon_host  ; }
  static int         GetPort    () { return m_mon_port  ; }
  static int         GetPort0   () { return m_mon_port_0; }
  static int         GetNumPorts() { return m_mon_n_port; }

  void StartServer();
  bool CloseExistingServer(const int port);
  static void* FuncServer(void* arg);
  void HandleConnection(TSocket* sock);
  void SetGoEnd(const bool val) { m_go_end = val; }
  bool GetGoEnd()        { return m_go_end; }

#ifndef __CINT__
  pthread_mutex_t* GetMutex() { return &mutex; }
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
