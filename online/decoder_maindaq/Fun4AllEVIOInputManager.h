#ifndef Fun4AllEVIOInputManager_H_
#define Fun4AllEVIOInputManager_H_

#include <fun4all/Fun4AllInputManager.h>

#include <string>
#include <map>

//class Event;
class PHCompositeNode;
class SyncObject;
//class ManageCodaInput;
class MainDaqParser;

class Fun4AllEVIOInputManager : public Fun4AllInputManager
{
 public:
  Fun4AllEVIOInputManager(const std::string &name = "DUMMY", const std::string &topnodename = "TOP");
  virtual ~Fun4AllEVIOInputManager();
  int fileopen(const std::string &filenam);
  int fileclose();
  int run(const int nevents = 0);
  int isOpen() {return isopen;}

  void Print(const std::string &what = "ALL") const;
  int ResetEvent();
  int PushBackEvents(const int i);
  int GetSyncObject(SyncObject **mastersync);
  int SyncIt(const SyncObject *mastersync);

 protected:
  int OpenNextFile();
  int segment;
  int isopen;
  int events_total;
  int events_thisfile;
  std::string topNodeName;
  PHCompositeNode *topNode;
  //Event *evt;
  //Event *save_evt;
  SyncObject* syncobject;
  //ManageCodaInput *coda;

 public:
  MainDaqParser* parser;
};

#endif /* __Fun4AllEVIOInputManager_H_ */
