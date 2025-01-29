#ifndef _FUN4ALL_UNIVERSAL_OUTPUT_MANAGER__H_
#define _FUN4ALL_UNIVERSAL_OUTPUT_MANAGER__H_
#include <fun4all/Fun4AllOutputManager.h>
#include <string>
#include <vector>
#include <map>
#include <TStopwatch.h>

class TFile;
class TTree;
class PHCompositeNode;
class SQEvent;
class SQSpillMap;
class SQHitVector;
class SQRun;

class Fun4AllRUSOutputManager : public Fun4AllOutputManager {
public:
    Fun4AllRUSOutputManager(const std::string &myname = "UNIVERSALOUT");
    virtual ~Fun4AllRUSOutputManager();

    void SetTreeName(const std::string& name) { m_tree_name = name; }
    void SetFileName(const std::string& name) { m_file_name = name; }
    virtual int Write(PHCompositeNode* startNode);
    void ResetBranches();
    void SetBasketSize(int size) { m_basket_size = size; }
    void SetAutoFlush(int flush) { m_auto_flush = flush; }
    void SetCompressionLevel(int level) { m_compression_level = level; }

protected:
    int OpenFile(PHCompositeNode* startNode);
    void CloseFile();

private:
    std::string m_tree_name;
    std::string m_file_name;
    std::string m_dir_base;
    bool m_dimuon_mode;
   
    TFile* m_file;
    TTree* m_tree;

    SQEvent* m_evt;
    SQSpillMap* m_sp_map;
    SQHitVector* m_hit_vec;
    SQRun* sq_run;  

    int m_basket_size;
    int m_auto_flush;
    int m_compression_level = 5;

    int runID;
    int spillID;
    int eventID;
    int turnID;
    int rfID;
    int rfIntensity[33];
    int fpgaTrigger[5] = {0};
    int nimTrigger[5] = {0};

    std::vector<int> hitID;
    std::vector<int> detectorID;
    std::vector<int> elementID;
    std::vector<double> tdcTime;
    std::vector<double> driftDistance;
};

#endif 
