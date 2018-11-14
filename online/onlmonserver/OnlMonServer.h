#pragma once
#include "HistoServer.h"
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

  void StartServer() {setup_server();}
#ifndef __CINT__
  void GetMutex(pthread_mutex_t &lock) {lock = mutex;}
  void SetThreadId(pthread_t &id) {serverthreadid = id;}
#endif
  void testmsg() {send_test_message();}
 protected:
  OnlMonServer(const std::string &name = "OnlMonServer");
#ifndef __CINT__
  pthread_mutex_t mutex;
  pthread_t serverthreadid;
#endif

};
