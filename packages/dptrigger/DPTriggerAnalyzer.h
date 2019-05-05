#ifndef DPTriggerAnalyzer_H
#define DPTriggerAnalyzer_H

// ROOT
#include <TString.h>

// Fun4All includes
#include <fun4all/SubsysReco.h>

// STL includes
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <list>
#include <map>
#include <string>

class PHG4Hit;
class SQRun;
class SQSpillMap;
class SQEvent;
class SQHit;
class SQHitMap;
class SQHitVector;
class PHG4HitContainer;
class SRecEvent;

class GeomSvc;


#define NTRPLANES 4
#define NMAXHODOS 16
#define NMAXMTRKS 1000
#define NMAXMHITS 1000000


class DPTriggerRoad
{
public:
    DPTriggerRoad();
    DPTriggerRoad(const std::list<int>& path);

    //! add one hit into the road
    void addTrElement(int uniqueID) { uniqueTrIDs.push_back(uniqueID); }
    void addTrElement(int detectorID, int elementID) { addTrElement(detectorID*1000 + elementID); }

    //!Get the sign of LR or TB
    int getTB();

    //!flip the LR or TB
    void flipTB();

    //!Other gets
    //@{
    int getRoadID() const { return roadID; }
    double getSigWeight() const { return sigWeight; }
    double getBkgRate() const { return bkgRate; }
    double getPxMin() const { return pXmin; }
    int getTrID(unsigned int i) const { return i < NTRPLANES ? uniqueTrIDs[i] : 0; }
    int getTrDetectorID(unsigned int i) const { return getTrID(i)/1000; }
    int getTrElementID(unsigned int i) const { return getTrID(i) % 1000; }
    TString getStringID();
    //@}

    //!Sets
    //@{
    void setRoadID(int id) { roadID = id; }
    void setSigWeight(double weight) { sigWeight = weight; }
    void setBkgRate(double rate) { bkgRate = rate; }
    void setPxMin(double pxmin) { pXmin = pxmin; }
    //@}

    //!comparison
    //@{
    bool operator == (const DPTriggerRoad& elem) const;
    bool operator <  (const DPTriggerRoad& elem) const;
    //@}

    //!printer
    friend std::ostream& operator << (std::ostream& os, const DPTriggerRoad& road);

private:

    //!unique road ID
    int roadID;

    //!total signal weight
    double sigWeight;

    //!total background occurence
    double bkgRate;

    //!Minimum Px
    double pXmin;

    //!unique detector element IDs: = 1000*detectorID + elementID
    std::vector<int> uniqueTrIDs;
};

class DPTriggerAnalyzer : public SubsysReco
{
public:
    //!Forward declaration of MatrixNode format -- TODO will be eventuall private
    class MatrixNode;

public:
    DPTriggerAnalyzer(const std::string &name = "DPTriggerAnalyzer");
    virtual ~DPTriggerAnalyzer();

#ifndef __CINT__
    int Init(PHCompositeNode *topNode);
#endif
    
    //! module initialization
    int InitRun(PHCompositeNode *topNode);
    
    //! event processing
    int process_event(PHCompositeNode *topNode);

    int End(PHCompositeNode *topNode);

    int ResetEvalVars();

  	const std::string& get_road_set_file_name() const {
  		return _road_set_file_name;
  	}

  	void set_road_set_file_name(const std::string& roadSetFileName) {
  		_road_set_file_name = roadSetFileName;
  	}

    const std::string& get_hit_container_choice() const {
      return _hit_container_type;
    }
    
    void set_hit_container_choice(const std::string& hitContainerChoice) {
      _hit_container_type = hitContainerChoice;
    }

    //!Build the trigger matrix by the input roads list
    void buildTriggerMatrix();

    //!Test the trigger pattern
    //void analyzeTrigger(DPMCRawEvent* rawEvent);

    //!create the trigger stations hit pattern, return false if one or more plane is missing
    bool buildHitPattern(int nTrHits, int uniqueTrIDs[]);

    //!search for possible roads
    void searchMatrix(MatrixNode* node, int level, int index);

    //!Tree deletion
    void deleteMatrix(MatrixNode* node);

    //!Helper function to retrieve the found road list
    std::list<DPTriggerRoad>& getRoadsFound(int index) { return roads_found[index]; }

    //!Helper functions to print various things
    void printHitPattern();
    void printPath();

private:

    int MakeNodes(PHCompositeNode *topNode);
    int GetNodes(PHCompositeNode *topNode);

    //! road set input file name
    std::string _road_set_file_name;

    std::string _hit_container_type;

    size_t _event;

    //!Internal hit pattern structure
    typedef std::vector<std::set<int> > TrHitPattern;
    TrHitPattern data;

    //!the trigger matrix, 0 for mu+, 1 for mu-
    //@{
    MatrixNode* matrix[2];
    std::map<TString, DPTriggerRoad> roads[2];
    //@}

    //!container of the roads found for +/-
    std::list<DPTriggerRoad> roads_found[2];

    //!temporary container of traversal path
    std::list<int> path;

    //!flag on NIM-ONLY analysis
    bool NIMONLY;

    SQRun* _run_header;
    SQSpillMap* _spill_map;
    SQEvent* _event_header;
    SQHitMap* _hit_map;
    SQHitVector* _hit_vector;
    SRecEvent* _recEvent;

    int run_id;
    int spill_id;
    int event_id;
    float target_pos;

    int Nhits_YNIM_1XT;
    int Nhits_YNIM_2XT;
    int Nhits_YNIM_3XT;
    int Nhits_YNIM_4XT;
    int Nhits_YNIM_1XB;
    int Nhits_YNIM_2XB;
    int Nhits_YNIM_3XB;
    int Nhits_YNIM_4XB;
    int Nhits_YNIM_1YL;
    int Nhits_YNIM_2YL;
    int Nhits_YNIM_4Y1L;
    int Nhits_YNIM_4Y2L;
    int Nhits_YNIM_1YR;
    int Nhits_YNIM_2YR;
    int Nhits_YNIM_4Y1R;
    int Nhits_YNIM_4Y2R;

    int n_FPGA_hits;
    int nPlusTop;
    int nPlusBot;
    int nMinusTop;
    int nMinusBot;
    int nHiPxPlusTop;
    int nHiPxPlusBot;
    int nHiPxMinusTop;
    int nHiPxMinusBot;

    bool HXT;
    bool HXB;
    bool HYL;
    bool HYR; 

    int uniqueIDs[NMAXMHITS];

    //!pointer to the digitizer, or geometry for that matter
    GeomSvc *p_geomSvc;
};

class DPTriggerAnalyzer::MatrixNode
{
public:
    MatrixNode(int uID);

    //!add a child
    void add(MatrixNode* child);

public:
    int uniqueID;
    std::list<MatrixNode*> children;
};

#endif
