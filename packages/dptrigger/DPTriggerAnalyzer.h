#ifndef DPTriggerAnalyzer_H
#define DPTriggerAnalyzer_H
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <list>
#include <map>
#include <string>
#include <TString.h>
#include <fun4all/SubsysReco.h>
class SQRun;
class SQEvent;
class SQHitVector;

#define NTRPLANES 4

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
    typedef enum { NIM_AND, NIM_OR } NimMode;
    DPTriggerAnalyzer(const std::string &name = "DPTriggerAnalyzer");
    virtual ~DPTriggerAnalyzer();

    int Init(PHCompositeNode *topNode);
    int InitRun(PHCompositeNode *topNode);
    int process_event(PHCompositeNode *topNode);
    int End(PHCompositeNode *topNode);

    const std::string& get_road_set_file_name() const {
      return _road_set_file_name;
    }
    
    void set_road_set_file_name(const std::string& roadSetFileName) {
      _road_set_file_name = roadSetFileName;
    }

    void set_nim_mode(const NimMode nim1, const NimMode nim2);
    
    //!Build the trigger matrix by the input roads list
    void buildTriggerMatrix();

    //!Test the trigger pattern
    //void analyzeTrigger(DPMCRawEvent* rawEvent);

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

    int GetNodes(PHCompositeNode *topNode);

    //! road set input file name
    std::string _road_set_file_name;
    NimMode _mode_nim1;
    NimMode _mode_nim2;

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
    SQEvent* _event_header;
    SQHitVector* _hit_vector;
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
