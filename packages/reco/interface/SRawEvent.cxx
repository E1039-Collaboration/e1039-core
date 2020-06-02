/*
SRawEvent.cxx

Implimention of the class SRawEvent

Author: Kun Liu, liuk@fnal.gov
Created: 10-24-2011
*/

#include <iostream>
#include <cmath>

#include <TRandom.h>
#include <TMath.h>
#include <TString.h>
#include <TROOT.h>
#include <TRandom.h>

#include "SRawEvent.h"

ClassImp(Hit)
ClassImp(SRawEvent)

Hit::Hit() : index(-1), detectorID(-1), flag(0)
{
}

Hit::Hit(int uniqueID)
{
    detectorID = getDetectorID(uniqueID);
    elementID = getElementID(uniqueID);
}

Hit::Hit(int dID, int eID) : detectorID(dID), elementID(eID)
{
}

bool Hit::operator<(const Hit& elem) const
{
    if(detectorID < elem.detectorID)
    {
        return true;
    }
    else if(detectorID > elem.detectorID)
    {
        return false;
    }

    if(elementID < elem.elementID)
    {
        return true;
    }
    else if(elementID > elem.elementID)
    {
        return false;
    }

    if(tdcTime > elem.tdcTime)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Hit::operator==(const Hit& elem) const
{
    if(detectorID == elem.detectorID && elementID == elem.elementID)
    {
        return true;
    }

    if(detectorID == elem.detectorID && fabs(pos - elem.pos) < 1E-3)
    {
        return true;
    }

    return false;
}

SRawEvent::SRawEvent() : fRunID(-1), fEventID(-1), fSpillID(-1), fTriggerBits(-1), fTriggerEmu(-1)
{
    fAllHits.clear();
    fTriggerHits.clear();
    for(Int_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++)
    {
        fNHits[i] = 0;
    }
}

SRawEvent::~SRawEvent()
{
}

void SRawEvent::DeepClone(SRawEvent *c)
{
    fRunID       = c->getRunID();
    fEventID     = c->getEventID();
    fSpillID     = c->getSpillID();

    fTriggerBits = c->getTriggerBits();

    fTargetPos   = c->getTargetPos();

    fTurnID      = c->getTurnID();
    fRFID        = c->getRFID();
    for(int i=0; i<33; ++i) fIntensity[i] = c->getIntensity(i);

    setTriggerEmu(c->isEmuTriggered());
    for(int i=0; i<4; ++i)  fNRoads[i]    = c->getNRoads()[i];


    for(Short_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++) {
        fNHits[i] = c->getNHitsInDetector(i);
    }
    fAllHits      = c->getAllHits();
    fTriggerHits  = c->getTriggerHits();

    return;
}

void SRawEvent::setEventInfo(Int_t runID, Int_t spillID, Int_t eventID)
{
    fRunID = runID;
    fEventID = eventID;
    fSpillID = spillID;
}

void SRawEvent::insertHit(Hit h)
{
    if(h.detectorID < 1 || h.detectorID > nChamberPlanes+nHodoPlanes+nPropPlanes) return;
    fAllHits.push_back(h);

    fNHits[0]++;
    fNHits[h.detectorID]++;
}

Int_t SRawEvent::findHit(Short_t detectorID, Short_t elementID)
{
    if(detectorID < 1 || detectorID > nChamberPlanes+nHodoPlanes+nPropPlanes) return -1;
    if(elementID < 0) return -1;

    /*
    This method produces problems in case of duplicate channels and thus people need to be cautious;
    It's okay here for two reasons:
       1. inTime is required when searching for trigger roads;
       2. hodoscope hit doesn't need tdcTime information as long as it's in-time;

    Please also note that this is valid only when the hit list is sorted.
    */

    Int_t idx_start = 0;
    for(int i = 1; i < detectorID; ++i) idx_start += fNHits[i];
    Int_t idx_end = idx_start + fNHits[detectorID];
    while(idx_start <= idx_end)
    {
        Int_t idx_mid = Int_t((idx_start + idx_end)/2);
        if(fAllHits[idx_mid].elementID == elementID)
        {
            return idx_mid;
        }
        else if(fAllHits[idx_mid].elementID < elementID)
        {
            idx_start = idx_mid + 1;
        }
        else
        {
            idx_end = idx_mid - 1;
        }
    }

    return -1;
}

Hit SRawEvent::getHit(Short_t detectorID, Short_t elementID)
{
    Int_t hitID = findHit(detectorID, elementID);
    if(hitID >= 0) return getHit(hitID);

    Hit dummy;
    return dummy;
}

std::list<Int_t> SRawEvent::getHitsIndexInDetector(Short_t detectorID)
{
    std::list<Int_t> hit_list;
    hit_list.clear();

    for(Int_t i = 0; i < fNHits[0]; i++)
    {
        if(fAllHits[i].detectorID != detectorID) continue;

        hit_list.push_back(i);
    }

    return hit_list;
}


std::list<Int_t> SRawEvent::getHitsIndexInDetector(Short_t detectorID, Double_t x_exp, Double_t win)
{
    std::list<Int_t> hit_list;
    hit_list.clear();

    for(Int_t i = 0; i < fNHits[0]; i++)
    {
        if(fAllHits[i].detectorID != detectorID) continue;
        if(fabs(fAllHits[i].pos - x_exp) > win) continue;

        hit_list.push_back(i);
    }

    return hit_list;
}

std::list<Int_t> SRawEvent::getHitsIndexInSuperDetector(Short_t detectorID)
{
    std::list<Int_t> hit_list;
    hit_list.clear();

    for(Int_t i = 0; i < fNHits[0]; i++)
    {
        if((fAllHits[i].detectorID != 2*detectorID) && (fAllHits[i].detectorID != 2*detectorID-1)) continue;

        hit_list.push_back(i);
    }

    return hit_list;
}

std::list<Int_t> SRawEvent::getHitsIndexInDetectors(std::vector<Int_t>& detectorIDs)
{
    std::list<Int_t> hit_list;
    hit_list.clear();

    UInt_t nDetectors = detectorIDs.size();
    for(Int_t i = 0; i < fNHits[0]; i++)
    {
        for(UInt_t j = 0; j < nDetectors; j++)
        {
            if(fAllHits[i].detectorID == detectorIDs[j])
            {
                hit_list.push_back(i);
                break;
            }
        }
    }

    return hit_list;
}

std::list<SRawEvent::hit_pair> SRawEvent::getPartialHitPairsInSuperDetector(Short_t detectorID)
{
    std::list<SRawEvent::hit_pair> _hitpairs;
    std::list<int> _hitlist1 = getHitsIndexInDetector(2*detectorID);
    std::list<int> _hitlist2 = getHitsIndexInDetector(2*detectorID - 1);

    std::vector<int> _hitflag1(_hitlist1.size(), -1);
    std::vector<int> _hitflag2(_hitlist2.size(), -1);

    //Temp solutions here, some number that is definitely larger than 0.5*spacing
    double spacing[(nChamberPlanes+nHodoPlanes+nPropPlanes)/2+1] =
                         {0., 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 1.3, 1.3, 1.3, 1.2, 1.2, 1.2, 1.2, 1.2, 1.2,  //DCs
                          4.0, 4.0, 7.0, 7.0, 8.0, 12.0, 12.0, 10.0,                          //hodos
                          3.0, 3.0, 3.0, 3.0};

    int index1 = -1;
    int index2 = -1;
    for(std::list<int>::iterator iter = _hitlist1.begin(); iter != _hitlist1.end(); ++iter)
    {
        index1++;
        index2 = -1;
        for(std::list<int>::iterator jter = _hitlist2.begin(); jter != _hitlist2.end(); ++jter)
        {
            index2++;
            if(fabs(fAllHits[*iter].pos - fAllHits[*jter].pos) > spacing[detectorID]) continue;
            _hitpairs.push_back(std::make_pair(*iter, *jter));

            _hitflag1[index1] = 1;
            _hitflag2[index2] = 1;
        }
    }

    index1 = 0;
    for(std::list<int>::iterator iter = _hitlist1.begin(); iter != _hitlist1.end(); ++iter)
    {
        if(_hitflag1[index1] < 0) _hitpairs.push_back(std::make_pair(*iter, -1));
        ++index1;
    }

    index2 = 0;
    for(std::list<int>::iterator iter = _hitlist2.begin(); iter != _hitlist2.end(); ++iter)
    {
        if(_hitflag2[index2] < 0) _hitpairs.push_back(std::make_pair(*iter, -1));
        ++index2;
    }

    return _hitpairs;
}

std::list<SRawEvent::hit_pair> SRawEvent::getPartialHitPairsInSuperDetector(Short_t detectorID, Double_t x_exp, Double_t win)
{
    std::list<SRawEvent::hit_pair> _hitpairs;
    std::list<int> _hitlist1 = getHitsIndexInDetector(2*detectorID, x_exp, win);
    std::list<int> _hitlist2 = getHitsIndexInDetector(2*detectorID - 1, x_exp, win+3);

    std::vector<int> _hitflag1(_hitlist1.size(), -1);
    std::vector<int> _hitflag2(_hitlist2.size(), -1);

    //Temp solutions here, some number that is definitely larger than 0.5*spacing
    double spacing[(nChamberPlanes+nHodoPlanes+nPropPlanes)/2+1] =
                         {0., 0.40, 0.40, 0.40, 0.40, 0.40, 0.40, 1.3, 1.3, 1.3, 1.2, 1.2, 1.2, 1.2, 1.2, 1.2,  //DCs
                          4.0, 4.0, 7.0, 7.0, 8.0, 12.0, 12.0, 10.0,                          //hodos
                          3.0, 3.0, 3.0, 3.0};                                                //prop tubes

    int index1 = -1;
    int index2 = -1;
    for(std::list<int>::iterator iter = _hitlist1.begin(); iter != _hitlist1.end(); ++iter)
    {
        index1++;
        index2 = -1;
        for(std::list<int>::iterator jter = _hitlist2.begin(); jter != _hitlist2.end(); ++jter)
        {
            index2++;
            if(fabs(fAllHits[*iter].pos - fAllHits[*jter].pos) > spacing[detectorID]) continue;
            _hitpairs.push_back(std::make_pair(*iter, *jter));

            _hitflag1[index1] = 1;
            _hitflag2[index2] = 1;
        }
    }

    index1 = 0;
    for(std::list<int>::iterator iter = _hitlist1.begin(); iter != _hitlist1.end(); ++iter)
    {
        if(_hitflag1[index1] < 0) _hitpairs.push_back(std::make_pair(*iter, -1));
        ++index1;
    }

    index2 = 0;
    for(std::list<int>::iterator iter = _hitlist2.begin(); iter != _hitlist2.end(); ++iter)
    {
        if(_hitflag2[index2] < 0) _hitpairs.push_back(std::make_pair(*iter, -1));
        ++index2;
    }

    return _hitpairs;
}

std::list<Int_t> SRawEvent::getAdjacentHitsIndex(Hit& _hit)
{
    std::list<Int_t> hit_list;
    hit_list.clear();

    Short_t detectorID = _hit.detectorID;
    Short_t detectorID_adj;
    if((detectorID/2)*2 == detectorID)
    {
        detectorID_adj = detectorID - 1;
    }
    else
    {
        detectorID_adj = detectorID + 1;
    }

    for(Int_t i = 0; i < fNHits[0]; i++)
    {
        if(fAllHits[i].detectorID == detectorID_adj && abs(fAllHits[i].elementID - _hit.elementID) <= 1)
        {
            hit_list.push_back(i);
        }
    }

    return hit_list;
}



Int_t SRawEvent::getNChamberHitsAll()
{
    Int_t nHits = 0;
    for(Int_t i = 1; i <= nChamberPlanes; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHodoHitsAll()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+1; i <= nChamberPlanes+nHodoPlanes; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNPropHitsAll()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+nHodoPlanes+1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInDetectors(std::vector<Int_t>& detectorIDs)
{
    Int_t nHits = 0;
    UInt_t nDetectors = detectorIDs.size();
    for(UInt_t i = 0; i < nDetectors; i++)
    {
        for(Int_t j = 0; j <= nChamberPlanes+nHodoPlanes+nPropPlanes; j++)
        {
            if(detectorIDs[i] == j)
            {
                nHits += fNHits[j];
                break;
            }
        }
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInD0()
{
    Int_t nHits = 0;
    for(Int_t i = 1; i <= 6; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInD1()
{
    Int_t nHits = 0;
    for(Int_t i = 7; i <= 12; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInD2()
{
    Int_t nHits = 0;
    for(Int_t i = 13; i <= 18; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInD3()
{
    return getNHitsInD3p() + getNHitsInD3m();
}

Int_t SRawEvent::getNHitsInD3p()
{
    Int_t nHits = 0;
    for(Int_t i = 19; i <= 24; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInD3m()
{
    Int_t nHits = 0;
    for(Int_t i = 25; i <= 30; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInH1()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+1; i <= nChamberPlanes+4; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInH2()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+5; i <= nChamberPlanes+8; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInH3()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+9; i <= nChamberPlanes+10; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInH4()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+11; i <= nChamberPlanes+nHodoPlanes; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInP1()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+nHodoPlanes+1; i <= nChamberPlanes+nHodoPlanes+4; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

Int_t SRawEvent::getNHitsInP2()
{
    Int_t nHits = 0;
    for(Int_t i = nChamberPlanes+nHodoPlanes+5; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i++)
    {
        nHits += fNHits[i];
    }

    return nHits;
}

void SRawEvent::reIndex(bool doSort)
{
    if(doSort) std::sort(fAllHits.begin(), fAllHits.end());

    ///Reset the number of hits on each plane
    for(Int_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++) fNHits[i] = 0;
    for(UInt_t i = 0; i < fAllHits.size(); i++) ++fNHits[fAllHits[i].detectorID];

    fNHits[0] = fAllHits.size();
}

void SRawEvent::mergeEvent(const SRawEvent& event)
{
    fAllHits.insert(fAllHits.end(), event.fAllHits.begin(), event.fAllHits.end());
    fTriggerHits.insert(fTriggerHits.end(), event.fTriggerHits.begin(), event.fTriggerHits.end());

    fTurnID = event.fTurnID;
    fRFID = event.fRFID;
    for(int i = 0; i < 33; ++i) fIntensity[i] = event.fIntensity[i];

    fTargetPos = event.fTargetPos;
    reIndex();
}

void SRawEvent::setEventInfo(SRawEvent* event)
{
    //Set runID, eventID, spillID
    setEventInfo(event->getRunID(), event->getSpillID(), event->getEventID());

    //Set trigger bits
    setTriggerBits(event->getTriggerBits());

    //Set target position
    setTargetPos(event->getTargetPos());

    //Set beam info
    setTurnID(event->getTurnID());
    setRFID(event->getRFID());
    setIntensity(event->getIntensityAll());

    //Set the trigger emu info
    setTriggerEmu(event->isEmuTriggered());
    setNRoads(event->getNRoads());
}

void SRawEvent::clear()
{
    //set everything to empty or impossible numbers
    fAllHits.clear();
    for(Int_t i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes+1; i++) fNHits[i] = 0;

    fRunID = -1;
    fSpillID = -1;
    fEventID = -1;

    fTriggerEmu = -1;
    for(int i = 0; i < 4; ++i) fNRoads[i] = 0;

    fTurnID = -1;
    fRFID = -1;
    for(int i = 0; i < 33; ++i) fIntensity[i] = -1;

    fTriggerBits = 0;
    fTriggerHits.clear();
}

void SRawEvent::setTriggerBits(Int_t triggers[])
{
    for(int i = 0; i < 10; ++i)
    {
        if(triggers[i] == 0) continue;
        fTriggerBits |= triggerBit(i);
    }
}

bool SRawEvent::isNIMTriggered()
{
    return isTriggeredBy(NIM1) || isTriggeredBy(NIM1) || isTriggeredBy(NIM3);
}

bool SRawEvent::isFPGATriggered()
{
    return isTriggeredBy(MATRIX1) || isTriggeredBy(MATRIX2) || isTriggeredBy(MATRIX3) || isTriggeredBy(MATRIX4) || isTriggeredBy(MATRIX5);
}

void SRawEvent::print(std::ostream& os) const
{
    os << "RunID: " << fRunID << ", EventID: " << fEventID << "===============" << std::endl;
    for(Int_t i = 1; i <= nChamberPlanes; i++)
    {
        os << "Layer " << i << " has " << fNHits[i] << " hits." << std::endl;
    }
    os << "===================================================================" << std::endl;

    return;
    for(auto iter = fAllHits.begin(); iter != fAllHits.end(); ++iter)
    {
        iter->print();
    }
    os << "===================================================================" << std::endl;
}
