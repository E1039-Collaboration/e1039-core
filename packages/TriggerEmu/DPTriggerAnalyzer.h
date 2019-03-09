#ifndef DPTriggerAnalyzer_H
#define DPTriggerAnalyzer_H

#include <fun4all/SubsysReco.h>

#include <iostream>
#include <vector>
#include <set>
#include <list>
#include <map>

#include <TString.h>

//#include "DPSimConfig.h"
//#include "DPMCRawEvent.h"
#include "DPDigitizer.h"

class PHG4Hit;
class SQHit;
class SQHitVector;
class PHG4HitContainer;

class GeomSvc;

#define SetBit(n) (1 << (n))
#define NTRPLANES 4

enum DPTriggerType
{
    MATRIX1 = SetBit(0),
    MATRIX2 = SetBit(1),
    MATRIX3 = SetBit(2),
    MATRIX4 = SetBit(3),
    MATRIX5 = SetBit(4),
    NIM1 = SetBit(5),
    NIM2 = SetBit(6),
    NIM3 = SetBit(7),
    NIM4 = SetBit(8),
    NIM5 = SetBit(9)
};

class DPTriggerRoad
{
public:
    DPTriggerRoad();
    DPTriggerRoad(const std::list<int>& path);

    //! add one hit into the road
    void addTrElement(int uniqueID) { uniqueTrIDs.push_back(uniqueID); }
    void addTrElement(int detectorID, int elementID) { addTrElement(detectorID*1000 + elementID); }

    //!Get the sign of LR or TB
    //int getLR();
    int getTB();

    //!flip the LR or TB
    //void flipLR();
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

    class DPDigitizer : public SubsysReco
    {
    public:
      DPDigitizer(const std::string &name = "DPDigitizer", const int verbo = 0);
      virtual ~DPDigitizer();

#ifndef __CINT__
	int Init(PHCompositeNode *topNode);
#endif

	//! module initialization
	int InitRun(PHCompositeNode *topNode);

	//! event processing
	int process_event(PHCompositeNode *topNode);

	//!main external call, fill the digi hit vector
	//void digitize(std::string detectorGroupName, PHG4Hit& g4hit);

	//!realization process
	//bool realize(SQHit& dHit);

	//!get the detectorID by detectorName
	//int getDetectorID(std::string detectorName) { return map_detectorID[detectorName]; }

	//!get the detectorName by detectorID
	//std::string getDetectorName(int detectorID) { return digiPlanes[detectorID].detectorName; }

	//!Get the trigger level by detectorID
	//int getTriggerLv(int detectorID) { return digiPlanes[detectorID].triggerLv; }

	//!Get the digi plane object by ID
	//DPDigiPlane& getDigiPlane(int detectorID) { return digiPlanes[detectorID]; }

	//!
	//std::string toGroupName(std::string in);

private:
    //!unique road ID
    int roadID;

    //!total signal weight
    double sigWeight;

    //!total background occurance
    double bkgRate;

    //!Minimum Px
    double pXmin;

    //!unique detector element IDs: = 1000*detectorID + elementID
    std::vector<int> uniqueTrIDs;
};

class DPTriggerAnalyzer
{
public:
    //!Forward declaration of MatrixNode format -- TODO will be eventuall private
    class MatrixNode;

public:
    DPTriggerAnalyzer();
    ~DPTriggerAnalyzer();

    static DPTriggerAnalyzer* instance();

    //!Build the trigger matrix by the input roads list
    void buildTriggerMatrix();

    //!Test the trigger pattern
    void analyzeTrigger(DPMCRawEvent* rawEvent);

    //!create the trigger stations hit pattern, return false if one or more plane is missing
    bool buildHitPattern(int nTrHits, int uniqueTrIDs[]);

    //!search for possible roads
    void searchMatrix(MatrixNode* node, int level, int index);

    //!Tree deletetion
    void deleteMatrix(MatrixNode* node);

    //!Helper function to retrieve the found road list
    std::list<DPTriggerRoad>& getRoadsFound(int index) { return roads_found[index]; }

    //!Helper functions to print various things
    void printHitPattern();
    void printPath();

private:
    static DPTriggerAnalyzer* p_triggerAna;

    //!pointer to the digitizer, or geometry for that matter
    DPDigitizer* p_digitizer;

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
