/*
EventReducer.h

This class is intended to handle all the hit list manipulation/reduction,
The reduction methods could initialized once for all, and then it will update
the hit list stored in SRawEvent. It is declared as a friend class of SRawEvent.

Author: Kun Liu, liuk@fnal.gov
Created: 06-17-2015
*/

#ifndef _EVENTREDUCER_H
#define _EVENTREDUCER_H

#include <GlobalConsts.h>

#include <list>
#include <map>
#include <TString.h>
#include <TRandom.h>

#include "SRawEvent.h"
#include "TriggerAnalyzer.h"

#include <phool/recoConsts.h>
#include <geom_svc/GeomSvc.h>

class EventReducer
{
public:
    EventReducer(TString options);
    ~EventReducer();

    //main external call
    int reduceEvent(SRawEvent* rawEvent);

    //external handle to set chamber efficiency/resolution
    void setChamEff(double val)   { chamEff = val;   }
    void setChamResol(double val) { chamResol = val; }

    //sagitta ratio reducer
    void sagittaReducer();

    //hough transform reducer
    void houghReducer();

    //hit cluster remover
    void deClusterize();
    void processCluster(std::vector<std::list<Hit>::iterator>& cluster);

    //hodosope maksing
    void initHodoMaskLUT();
    void hodoscopeMask(std::list<Hit>& chamberhits, std::list<Hit>& hodohits);
    bool lineCrossing(double x1, double y1, double x2, double y2,
                      double x3, double y3, double x4, double y4);

private:
    //pointer to geometry service, inited outside
    GeomSvc* p_geomSvc;

    //pointer to the reco configuration 
    recoConsts* rc;

    //pointer to trigger analyzer, inited inside
    TriggerAnalyzer* p_triggerAna;

    //Random number
    TRandom rndm;

    //temporary container for the hit list
    std::list<Hit> hitlist;
    std::list<Hit> hodohitlist;

    //loop-up table of hodoscope masking
    typedef std::map<int, std::vector<int> > LUT;
    LUT h2celementID_lo;
    LUT h2celementID_hi;
    LUT c2helementIDs;

    //flags of the hit manipulation method
    bool afterhit;            //after pulse removal
    bool hodomask;            //hodoscope masking
    bool outoftime;           //out of time hit removal
    bool decluster;           //remove hit clusters in chamber
    bool mergehodo;           //merge trigger hit with hit
    bool triggermask;         //use active trigger road for track masking
    bool sagitta;             //remove the hits which cannot form a sagitta triplet
    bool hough;               //remove the hits which cannot form a peak in hough space, will be implemented later
    bool realization;         //apply detector efficiency and resolution by dropping and smear
    bool difnim;              //treat the nim/FPGA triggered events differently, i.e. no trigger masking in NIM events

    //Adjustable parameters
    double timeOffset;        //timing correction
    double chamEff;           //chamber efficiency
    double chamResol;         //chamber resolution

    //SagittaReducer parameters
    double SAGITTA_DUMP_CENTER;
    double SAGITTA_DUMP_WIDTH;
    double SAGITTA_TARGET_CENTER;
    double SAGITTA_TARGET_WIDTH;
    double Z_TARGET;
    double Z_DUMP;

    //Hodo masking parameters
    double TX_MAX;
    double TY_MAX;
    bool USE_V1495_HIT;
    bool USE_TWTDC_HIT;
};

#endif
