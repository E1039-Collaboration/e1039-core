#ifndef _TRIGGERROAD_H
#define _TRIGGERROAD_H

#include <iostream>
#include <vector>
#include <list>
#include <cmath>

#include <TObject.h>
#include <TROOT.h>
#include <TH1D.h>

#include "FastTracklet.h"
#include "SRecEvent.h"

class TriggerRoad : public TObject
{
public:
    TriggerRoad();
    TriggerRoad(std::list<int> uniqueIDs);
    TriggerRoad(Tracklet& track);
    TriggerRoad(SRecTrack& track);

    //Flag
    bool isValid();
    bool isEnabled() { return enabled; }

    //Get reflection in T/B, LR
    TriggerRoad reflectTB();
    TriggerRoad reflectLR();

    //Insert a hodo paddle into the road
    void addElement(int detectorID, int elementID);

    //Setters
    void enable() { enabled = true; }
    void disable() { enabled = false; }

    //Getters
    int getNElements() { return detectorIDs.size(); }
    int getDetectorID(int i) { return detectorIDs[i]; }
    int getElementID(int i) { return elementIDs[i]; }
    int getUniqueID(int i) { return detectorIDs[i]*100 + elementIDs[i]; }

    int getRoadID();

    //The total weight and ratio of target weight
    double weight() const { return targetWeight + dumpWeight; }
    double ratio() const { return targetWeight/(targetWeight + dumpWeight); }
    double mratio() const { return highMWeight/(lowMWeight + highMWeight); }

    //Mean and sigma of pX distribution
    int getNEntries() const { return pXs.size(); }
    double getpXMean() const;
    double getpXWidth() const;

    //T/B , L/R
    int getTB();
    int getLR();

    //counts of mu+, mu-
    int nPlus;
    int nMinus;

    //print
    void print();

    //Clear the container
    void clear();

    //Overriden operators for comparison and sort
    bool operator==(const TriggerRoad& elem) const;

    static bool byTargetDump(const TriggerRoad& elem1, const TriggerRoad& elem2);
    static bool byWeight(const TriggerRoad& elem1, const TriggerRoad& elem2);
    static bool byMass(const TriggerRoad& elem1, const TriggerRoad& elem2);
    static bool byPt(const TriggerRoad& elem1, const TriggerRoad& elem2);
    static bool byRndFrequency(const TriggerRoad& elem1, const TriggerRoad& elem2);

    //overload operator +=
    TriggerRoad& operator+=(const TriggerRoad& elem);

    //Road maker
    static std::list<TriggerRoad> makeRoadList(int nHits, int dIDs[], int eIDs[], double z, double mass, double pX, double weight);

    //overload stream operator <<
    friend std::ostream& operator << (std::ostream& os, const TriggerRoad& road);

public:
    //Unique road ID
    int roadID;

    //Lv1 group ID
    int groupID;

    //Weights
    double targetWeight;
    double dumpWeight;
    double lowMWeight;
    double highMWeight;

    //pX distributions
    std::vector<double> pXs;
    double px_min;
    double px_max;
    double px_mean;

    //Random frequency
    double rndf;

    //Flag
    bool enabled;

    //hodo paddles
    std::vector<int> detectorIDs;
    std::vector<int> elementIDs;

    ClassDef(TriggerRoad, 2)
};

typedef std::pair<int, int> Trigger;

#endif
