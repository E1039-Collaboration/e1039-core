#include "OnlMonServer.h"

using namespace std;

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

