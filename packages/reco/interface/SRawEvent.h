/*
SRawEvent.h

Definition of the class SRawEvent, which essentially works as a
container of the raw hits information. It also provides serveral
query utility to retrieve the hit list from a specific plane, etc.

Author: Kun Liu, liuk@fnal.gov
Created: 07-02-2012
*/

#ifndef _SRAWEVENT_H
#define _SRAWEVENT_H

#include <GlobalConsts.h>

#include <phool/PHObject.h>

#include <iostream>
#include <vector>
#include <list>
#include <string>

#include <TObject.h>
#include <TROOT.h>
#include <TVector3.h>

#define triggerBit(n) (1 << (n))
#define hitFlagBit(n) (1 << (n))

class EventReducer;

///Definition of hit structure
class Hit: public PHObject
{
public:
    //Constructor
    Hit();
    Hit(int uniqueID);
    Hit(int detectorID, int elementID);

    /// PHObject virtual overloads
    void         identify(std::ostream& os = std::cout) const {print(os);}
    void         Reset() {*this = Hit();}
    int          isValid() const {return true;}
    Hit*         Clone() const {return (new Hit(*this));}

    //Decompose the data quality flag
    bool isInTime() const { return (flag & Hit::inTime) != 0; }
    bool isHodoMask() const { return (flag & Hit::hodoMask) != 0; }
    bool isTriggerMask() const { return (flag & Hit::triggerMask) != 0; }

    //Set the flag
    void setFlag(UShort_t flag_input) { flag |= flag_input; }
    void resetFlag(UShort_t flag_input) { flag &= ~flag_input; }
    void setInTime(bool f = true) { f ? (flag |= inTime) : (flag &= ~inTime); }
    void setHodoMask(bool f = true) { f ? (flag |= hodoMask) : (f &= ~hodoMask); }
    void setTriggerMask(bool f = true) { f ? (flag |= triggerMask) : (flag &= ~triggerMask); }

    //Sign of this hit
    Int_t getSign() { return driftDistance > 0 ? 1 : -1; }

    //convert detectorID and elementID into one number
    Int_t uniqueID() { return detectorID*1000 + elementID; }
    Int_t getDetectorID(Int_t uniqueID) { return uniqueID/1000; }
    Int_t getElementID(Int_t uniqueID) { return uniqueID % 1000; }

    //overridden comparison operator for track seeding
    bool operator<(const Hit& elem) const;
    bool operator==(const Hit& elem) const;

    //Debugging output
    void print(std::ostream& os = std::cout) const
    { os << index << " : " << detectorID << " : " << elementID << " : " << pos << " : " << driftDistance << " : " << isInTime() << " : " << isHodoMask() << " : " << isTriggerMask() << std::endl; }

    //Data members
    Int_t index;             //unique index for identification
    Short_t detectorID;      //assigned for each detector plane
    Short_t elementID;
    Float_t tdcTime;         //raw TDC time
    Float_t driftDistance;
    Float_t pos;             //actual measurement in either X, Y, U or V direction

    //hit quality flag
    enum hitQuality
    {
        inTime = hitFlagBit(1),
        hodoMask = hitFlagBit(2),
        triggerMask = hitFlagBit(3)
    };
    UShort_t flag;

    ClassDef(Hit, 4)
};

class SRawEvent: public PHObject
{
public:
    SRawEvent();
    ~SRawEvent();

    /// PHObject virtual overloads
    void         identify(std::ostream& os = std::cout) const { print(os);}
    void         Reset() {*this = SRawEvent();}
    int          isValid() const {return true;}
    SRawEvent*   Clone() const {return (new SRawEvent(*this));}

    void         DeepClone(SRawEvent *c);

    ///Gets
    //Hit lists
    std::list<Int_t> getHitsIndexInDetector(Short_t detectorID);
    std::list<Int_t> getHitsIndexInDetector(Short_t detectorID, Double_t x_exp, Double_t win);
    std::list<Int_t> getHitsIndexInSuperDetector(Short_t detectorID);
    std::list<Int_t> getHitsIndexInDetectors(std::vector<Int_t>& detectorIDs);
    std::list<Int_t> getAdjacentHitsIndex(Hit& _hit);

    Int_t getNHitsAll() { return fNHits[0]; }
    Int_t getNTriggerHits() { return fTriggerHits.size(); }
    Int_t getNChamberHitsAll();
    Int_t getNHodoHitsAll();
    Int_t getNPropHitsAll();

    Int_t getNHitsInD0();
    Int_t getNHitsInD1();
    Int_t getNHitsInD2();
    Int_t getNHitsInD3();
    Int_t getNHitsInD3p();
    Int_t getNHitsInD3m();
    Int_t getNHitsInH1();
    Int_t getNHitsInH2();
    Int_t getNHitsInH3();
    Int_t getNHitsInH4();
    Int_t getNHitsInP1();
    Int_t getNHitsInP2();

    Int_t getNHitsInDetector(Short_t detectorID) { return fNHits[detectorID]; }
    Int_t getNHitsInSuperDetector(Short_t detectorID) { return fNHits[2*detectorID-1] + fNHits[2*detectorID]; }
    Int_t getNHitsInDetectors(std::vector<Int_t>& detectorIDs);

    std::vector<Hit>& getAllHits() { return fAllHits; }
    std::vector<Hit>& getTriggerHits() { return fTriggerHits; }
    Hit getTriggerHit(Int_t index) { return fTriggerHits[index]; }
    Hit getHit(Int_t index) { return fAllHits[index]; }
    Hit getHit(Short_t detectorID, Short_t elementID);
    void setHitFlag(Int_t index, Short_t flag) { if(index < 0) return; fAllHits[index].setFlag(flag); }
    void setHitFlag(Short_t detectorID, Short_t elementID, Short_t flag) { setHitFlag(findHit(detectorID, elementID), flag); }

    Int_t getRunID() { return fRunID; }
    Int_t getEventID() { return fEventID; }
    Int_t getSpillID() { return fSpillID; }

    ///Sets
    void setEventInfo(Int_t runID, Int_t spillID, Int_t eventID);
    void setHit(Int_t index, Hit h) { fAllHits[index] = h; }
    void setTriggerHit(Int_t index, Hit h) { fTriggerHits[index] = h; }

    ///Insert a new hit
    void insertHit(Hit h);
    void insertTriggerHit(Hit h) { if(h.detectorID >= nChamberPlanes+1 && h.detectorID <= nChamberPlanes+nHodoPlanes) fTriggerHits.push_back(h); }

    ///Find a hit -- binary search since hit list is sorted
    Int_t findHit(Short_t detectorID, Short_t elementID);

    ///Reset the number hits on each plane
    void reIndex(bool doSort = false);

    ///Type of pair with two adjacent wires
    typedef std::pair<Int_t, Int_t> hit_pair;
    std::list<SRawEvent::hit_pair> getPartialHitPairsInSuperDetector(Short_t detectorID);
    std::list<SRawEvent::hit_pair> getPartialHitPairsInSuperDetector(Short_t detectorID, Double_t x_exp, Double_t wind);

    ///Set/get the trigger types
    Int_t getTriggerBits() { return fTriggerBits; }
    void setTriggerBits(Int_t triggers[]);
    void setTriggerBits(Int_t triggers) { fTriggerBits = triggers; }
    bool isTriggeredBy(Int_t trigger) { return (fTriggerBits & trigger) != 0; }
    bool isNIMTriggered();
    bool isFPGATriggered();

    //Set/get offline trigger emulation results
    bool isEmuTriggered() { return fTriggerEmu > 0; }
    Int_t getNRoadsPos() { return fNRoads[0] + fNRoads[1]; }
    Int_t getNRoadsNeg() { return fNRoads[2] + fNRoads[3]; }
    Int_t getNRoadsPosTop() { return fNRoads[0]; }
    Int_t getNRoadsPosBot() { return fNRoads[1]; }
    Int_t getNRoadsNegTop() { return fNRoads[2]; }
    Int_t getNRoadsNegBot() { return fNRoads[3]; }
    Short_t* getNRoads() { return fNRoads; }
    void setTriggerEmu(bool flag) { fTriggerEmu = flag ? 1 : -1; }
    void setNRoads(Short_t nRoads[]) { for(Int_t i = 0; i < 4; ++i) fNRoads[i] = nRoads[i]; }
    void setNRoads(Int_t nRoads[]) { for(Int_t i = 0; i < 4; ++i) fNRoads[i] = nRoads[i]; }

    //Set/get the target position
    Int_t getTargetPos() { return fTargetPos; }
    void setTargetPos(Short_t targetPos) { fTargetPos = targetPos; }

    //Set/get the beam info
    Int_t getTurnID() { return fTurnID; }
    Int_t getRFID() { return fRFID; }
    Int_t getIntensity() { return fIntensity[16]; }
    Int_t getIntensity(Int_t i) { return fIntensity[i+16]; }
    Int_t getIntensitySumBefore(Int_t n = 16) { Int_t sum = 0; for(Int_t i = n; i < 16; ++i) sum += fIntensity[i]; return sum; }
    Int_t getIntensitySumAfter(Int_t n = 16) { Int_t sum = 0; for(Int_t i = 16; i < n+16; ++i) sum += fIntensity[i]; return sum; }
    Int_t* getIntensityAll() { return fIntensity; }

    void setTurnID(Int_t turnID) { fTurnID = turnID; }
    void setRFID(Int_t rfID) { fRFID = rfID; }
    void setIntensity(const Int_t intensity[]) { for(Int_t i = 0; i < 33; ++i) fIntensity[i] = intensity[i]; }
    void setIntensity(Int_t i, Int_t val) { fIntensity[i] = val; }
    void setIntensity(Int_t val) { fIntensity[16] = val; }

    ///Merge a event to this event
    void mergeEvent(const SRawEvent& rawEvent);

    ///Set the event info from another event
    void setEventInfo(SRawEvent* event);

    ///Clear the internal event structure
    void clear();

    ///only empty the hit list, leave other information untouched
    void empty() { fAllHits.clear(); fTriggerHits.clear(); }
    void emptyHits() { fAllHits.clear(); }
    void emptyTriggerHits() { fTriggerHits.clear(); }

    ///Print for debugging purposes
    void print (std::ostream& os = std::cout) const;

    ///Friend class which handles all kinds of hit list reduction
    friend class EventReducer;

public:
    //Trigger type
    enum TriggerType
    {
        MATRIX1 = triggerBit(0),
        MATRIX2 = triggerBit(1),
        MATRIX3 = triggerBit(2),
        MATRIX4 = triggerBit(3),
        MATRIX5 = triggerBit(4),
        NIM1 = triggerBit(5),
        NIM2 = triggerBit(6),
        NIM3 = triggerBit(7),
        NIM4 = triggerBit(8),
        NIM5 = triggerBit(9)
    };

private:
    //RunID, spillID, eventID
    Int_t fRunID;
    Int_t fEventID;
    Int_t fSpillID;

    //Trigger bit
    Int_t fTriggerBits;

    //Target pos
    Short_t fTargetPos;

    //Beam intensity information
    Int_t fTurnID;
    Int_t fRFID;
    Int_t fIntensity[33];   //16 before, one onset, and 16 after

    //Offline trigger simulation res
    Short_t fTriggerEmu;
    Short_t fNRoads[4];       //0, positive top; 1, positive bottom; 2, negative top; 3, negative bottom

    ///Hits of this event
    Int_t fNHits[nChamberPlanes+nHodoPlanes+nPropPlanes+nDarkPhotonPlanes+1];  //0 for all hits, 1, 2, ..., 24 for number of hits in plane 1, 2, ..., 24
    std::vector<Hit> fAllHits;
    std::vector<Hit> fTriggerHits;

    ClassDef(SRawEvent, 9)
};

#endif
