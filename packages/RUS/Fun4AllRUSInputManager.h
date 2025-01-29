#ifndef Fun4AllRUSInputManager_H_
#define Fun4AllRUSInputManager_H_
#include <fun4all/Fun4AllInputManager.h>
#include <string>
#include <vector>
#include <map>

class PHCompositeNode;
class SyncObject;

class TFile;
class TTree;
class SQRun;
class SQSpillMap;
class SQEvent;
class SQHitVector;

class Fun4AllRUSInputManager : public Fun4AllInputManager {
public:
    Fun4AllRUSInputManager(const std::string &name = "DUMMY", const std::string &topnodename = "TOP");
    virtual ~Fun4AllRUSInputManager();

    int fileopen(const std::string &filename);
    int fileclose();
    int run(const int nevents = 0);
    int isOpen() { return isopen; }

    void Print(const std::string &what = "ALL") const;
    int ResetEvent();
    int PushBackEvents(const int i);
    int GetSyncObject(SyncObject **mastersync);
    int SyncIt(const SyncObject *mastersync);

    const std::string& get_branch_name() const { return _branch_name; }
    void set_branch_name(const std::string& branchName) { _branch_name = branchName; }

    const std::string& get_tree_name() const { return _tree_name; }
    void set_tree_name(const std::string& treeName) { _tree_name = treeName; }
protected:
    int OpenNextFile();
    void VectToE1039();
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
    int runID;
    int spillID;
    int eventID;
    int turnID;
    int rfID;
    int rfIntensity[33] = {0};
    int fpgaTrigger[5] = {0};
    int nimTrigger[5] = {0};

    std::vector<int>* hitID = nullptr;
    std::vector<int>* detectorID = nullptr;
    std::vector<int>* elementID = nullptr;
    std::vector<double>* driftDistance = nullptr;
    std::vector<double>* tdcTime = nullptr;

    SQRun*       run_header;
    SQSpillMap*  spill_map;
    SQEvent*     event_header;
    SQHitVector* hit_vec;
};

#endif /* __Fun4AllRUSInputManager_H_ */
