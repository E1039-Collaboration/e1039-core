#ifndef Fun4AllSRawEventInputManager_H_
#define Fun4AllSRawEventInputManager_H_

#include <fun4all/Fun4AllInputManager.h>

#include <string>
#include <map>


class PHCompositeNode;
class SyncObject;

class TFile;
class TTree;
class SRawEvent;

class Fun4AllSRawEventInputManager : public Fun4AllInputManager
{
 public:
  Fun4AllSRawEventInputManager(const std::string &name = "DUMMY", const std::string &topnodename = "TOP");
  virtual ~Fun4AllSRawEventInputManager();
  int fileopen(const std::string &filenam);
  int fileclose();
  int run(const int nevents = 0);
  int isOpen() {return isopen;}

  void Print(const std::string &what = "ALL") const;
  int ResetEvent();
  int PushBackEvents(const int i);
  int GetSyncObject(SyncObject **mastersync);
  int SyncIt(const SyncObject *mastersync);

	const std::string& get_branch_name() const {
		return _branch_name;
	}

	void set_branch_name(const std::string& branchName) {
		_branch_name = branchName;
	}

	const std::string& get_tree_name() const {
		return _tree_name;
	}

	void set_tree_name(const std::string& treeName) {
		_tree_name = treeName;
	}

 protected:
  int OpenNextFile();
  int segment;
  int isopen;
  int events_total;
  int events_thisfile;
  std::string topNodeName;
  PHCompositeNode *topNode;
  SyncObject* syncobject;

  std::string _tree_name;
  std::string _branch_name;

  TFile* _fin;
  TTree* _tin;
  SRawEvent* _b_srawEvent;
};

#endif /* __Fun4AllSRawEventInputManager_H_ */
