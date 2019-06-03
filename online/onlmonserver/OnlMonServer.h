#pragma once
#include "HistoServer.h"
#include <vector>
#include <fun4all/Fun4AllServer.h>
#include <pthread.h>

class Fun4AllInputManager;
class Fun4AllSyncManager;
class Fun4AllOutputManager;
class PHCompositeNode;
class PHTimeStamp;
class SubsysReco;
class TDirectory;
class TH1;
class TNamed;

class OnlMonServer : public Fun4AllServer
{
 public:
  static OnlMonServer *instance();
  virtual ~OnlMonServer();

  static void SetServer(const std::string server);
  static void SetPort  (const int port);
  static std::string GetServer();
  static int         GetPort  ();

  void StartServer() {setup_server();}
#ifndef __CINT__
  void GetMutex(pthread_mutex_t &lock) {lock = mutex;}
  void SetThreadId(pthread_t &id) {serverthreadid = id;}
#endif
  void testmsg() {send_test_message();}
  void testhist() {receive_hist_all();}
  int MonitorSubsys(const char* name, std::vector<TH1*>& hist_list) { return monitor_subsys(name, hist_list); }
 protected:
  OnlMonServer(const std::string &name = "OnlMonServer");
#ifndef __CINT__
  pthread_mutex_t mutex;
  pthread_t serverthreadid;
#endif

};
