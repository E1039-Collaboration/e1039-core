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
class SQTrackVector;
class SQRun;

class Fun4AllRUSOutputManager : public Fun4AllOutputManager {
    public:
        Fun4AllRUSOutputManager(const std::string &myname = "UNIVERSALOUT");
        virtual ~Fun4AllRUSOutputManager();

        int process_id;
        int source_flag;
        bool true_mode;
        bool exp_mode;
        bool sqevent_flag;

        void SetTreeName(const std::string& name) { m_tree_name = name; }
        void SetFileName(const std::string& name) { m_file_name = name; }
        virtual int Write(PHCompositeNode* startNode);
        void ResetHitBranches();
        void ResetTrueTrackBranches();
        void SetBasketSize(int size) { m_basket_size = size; }
        void SetAutoFlush(int flush) { m_auto_flush = flush; }
        void SetCompressionLevel(int level) { m_compression_level = level; }
        unsigned int EncodeProcess(int processID, int sourceFlag);
        void SetProcessId(int proc_id) { process_id = proc_id; }
        void DisableSQEvent(bool enable) { sqevent_flag = enable; }

        void SetMCTrueTrackMode(bool enable) {
            true_mode = enable;
            if (enable) {
                exp_mode = false;
            }
        }

        void SetExpMode(bool enable) {
            exp_mode = enable;
            if (enable) {
                true_mode = false;
            }
        }

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
        SQTrackVector * m_vec_trk;
        SQRun* sq_run;  

        int m_basket_size;
        int m_auto_flush;
        int m_compression_level;

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

        std::vector<int> hitTrackID, gCharge, gTrackID, gProcessID;
        std::vector<double> gvx, gvy, gvz, gpx, gpy, gpz;
        std::vector<double> gx_st1, gy_st1, gz_st1, gpx_st1, gpy_st1, gpz_st1;
        std::vector<double> gx_st3, gy_st3, gz_st3, gpx_st3, gpy_st3, gpz_st3;

};

#endif 
