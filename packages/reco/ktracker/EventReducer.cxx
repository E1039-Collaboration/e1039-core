#include <iostream>
#include <stdlib.h>

#include "EventReducer.h"

EventReducer::EventReducer(TString options) : afterhit(false), hodomask(false), outoftime(false), decluster(false), mergehodo(false), triggermask(false), sagitta(false), hough(false), realization(false), difnim(false)
{
    //parse the reducer setup
    options.ToLower();
    if(options.Contains("a")) afterhit = true;
    if(options.Contains("h")) hodomask = true;
    if(options.Contains("o")) outoftime = true;
    if(options.Contains("c")) decluster = true;
    if(options.Contains("m")) mergehodo = true;
    if(options.Contains("t")) triggermask = true;
    if(options.Contains("s")) sagitta = true;
    if(options.Contains("g")) hough = true;
    if(options.Contains("r")) realization = true;
    if(options.Contains("n")) difnim = true;

    if(options.Contains("e")) {
      std::cout << "EventReducer: !!ERROR!!  Option 'e' is no longer available.  Use 'CalibDriftDist'.  Abort." << std::endl;
      exit(1);
    }

    rc = recoConsts::instance();
    timeOffset = rc->get_DoubleFlag("TDCTimeOffset");
    SAGITTA_TARGET_CENTER = rc->get_DoubleFlag("SAGITTA_TARGET_CENTER");
    SAGITTA_TARGET_WIDTH = rc->get_DoubleFlag("SAGITTA_TARGET_WIDTH");
    SAGITTA_DUMP_CENTER = rc->get_DoubleFlag("SAGITTA_DUMP_CENTER");
    SAGITTA_TARGET_WIDTH = rc->get_DoubleFlag("SAGITTA_DUMP_WIDTH");
    Z_TARGET = rc->get_DoubleFlag("Z_TARGET");
    Z_DUMP = rc->get_DoubleFlag("Z_DUMP");

    TX_MAX = rc->get_DoubleFlag("TX_MAX");
    TY_MAX = rc->get_DoubleFlag("TY_MAX");
    USE_V1495_HIT = rc->get_BoolFlag("USE_V1495_HIT");
    USE_TWTDC_HIT = rc->get_BoolFlag("USE_TWTDC_HIT");
    
    chamEff    = 0.94;
    chamResol  = 0.04;

    //Screen output for all the methods enabled
    if(afterhit)      std::cout << "EventReducer: after-pulse removal enabled. " << std::endl;
    if(hodomask)      std::cout << "EventReducer: hodoscope masking enabled. " << std::endl;
    if(outoftime)     std::cout << "EventReducer: out-of-time hits removal enabled. " << std::endl;
    if(decluster)     std::cout << "EventReducer: hit cluster removal enabled. " << std::endl;
    if(mergehodo)     std::cout << "EventReducer: v1495 hits will be merged with TW-TDC hits. " << std::endl;
    if(triggermask)   std::cout << "EventReducer: trigger road masking enabled. " << std::endl;
    if(sagitta)       std::cout << "EventReducer: sagitta reducer enabled. " << std::endl;
    if(hough)         std::cout << "EventReducer: hough transform reducer enabled. " << std::endl;
    if(realization)   std::cout << "EventReducer: realization enabled. " << std::endl;
    if(difnim)        std::cout << "EventReducer: trigger masking will be disabled in NIM events. " << std::endl;
    if(fabs(timeOffset) > 0.01) std::cout << "EventReducer: " << timeOffset << " ns will be added to tdcTime. " << std::endl;

    //initialize services
    p_geomSvc = GeomSvc::instance();
    if(triggermask)
    {
        p_triggerAna = new TriggerAnalyzer();
        p_triggerAna->init();
        p_triggerAna->buildTriggerTree();
    }

    if(hodomask) initHodoMaskLUT();

    //set random seed
    rndm.SetSeed(0);
}

EventReducer::~EventReducer()
{
    if(triggermask)
    {
        delete p_triggerAna;
    }
}

int EventReducer::reduceEvent(SRawEvent* rawEvent)
{
    int nHits_before = rawEvent->getNChamberHitsAll();

    //temporarily disable trigger road masking if this event is not fired by any MATRIX triggers
    bool triggermask_local = triggermask;
    if(!(rc->get_BoolFlag("MC_MODE") || rawEvent->isFPGATriggered()))
    {
        triggermask_local = false;
    }

    //dump the vector of hits from SRawEvent to a list first
    hitlist.clear();
    hodohitlist.clear();
    for(std::vector<Hit>::iterator iter = rawEvent->fAllHits.begin(); iter != rawEvent->fAllHits.end(); ++iter)
    {
        if(outoftime && (!iter->isInTime())) continue;

        if(iter->detectorID <= nChamberPlanes)    //chamber hits
        {
            if(realization && rndm.Rndm() > chamEff) continue;
            //if(hodomask && (!iter->isHodoMask())) continue;
            //if(triggermask && (!iter->isTriggerMask())) continue;
        }
        else if(iter->detectorID > nChamberPlanes && iter->detectorID <= nChamberPlanes+nHodoPlanes)
        {
            // if trigger masking is enabled, all the X hodos are discarded
            if(triggermask_local && p_geomSvc->getPlaneType(iter->detectorID) == 1) continue;
        }

        if(realization && iter->detectorID <= nChamberPlanes) iter->driftDistance += rndm.Gaus(0., chamResol);

        if(iter->detectorID >= nChamberPlanes+1 && iter->detectorID <= nChamberPlanes+nHodoPlanes)
        {
            hodohitlist.push_back(*iter);
        }
        else
        {
            hitlist.push_back(*iter);
        }
    }

    if(mergehodo)
    {
        for(std::vector<Hit>::iterator iter = rawEvent->fTriggerHits.begin(); iter != rawEvent->fTriggerHits.end(); ++iter)
        {
            if(triggermask_local && p_geomSvc->getPlaneType(iter->detectorID) == 1) continue;
            hodohitlist.push_back(*iter);
        }
    }

    // manully create the X-hodo hits by the trigger roads
    if(triggermask_local) p_triggerAna->trimEvent(rawEvent, hodohitlist, mergehodo || USE_V1495_HIT, mergehodo || USE_TWTDC_HIT);

    //apply hodoscope mask
    hodohitlist.sort();
    if(hodomask) hodoscopeMask(hitlist, hodohitlist);

    //Remove after hits
    hitlist.sort();
    hitlist.merge(hodohitlist);
    if(afterhit) hitlist.unique();

    //Remove hit clusters
    if(decluster) deClusterize();

    //Remove the hits by sagitta ratio
    if(sagitta) sagittaReducer();

    //Push everything back to SRawEvent
    rawEvent->fAllHits.clear();
    rawEvent->fAllHits.assign(hitlist.begin(), hitlist.end());

    rawEvent->reIndex();
    return nHits_before - rawEvent->getNChamberHitsAll();
}

void EventReducer::sagittaReducer()
{
    //find index for D1, D2, and D3
    int nHits_D1 = 0;
    int nHits_D2 = 0;
    int nHits_D3 = 0;
    int detectorID_st1_max = 12;
    int detectorID_st2_max = 18;

    //hitlist here is assumed to be sorted of course
    for(std::list<Hit>::iterator iter = hitlist.begin(); iter != hitlist.end(); ++iter)
    {
        if(iter->detectorID > nChamberPlanes) break;
        if(iter->detectorID <= detectorID_st1_max)
        {
            ++nHits_D1;
        }
        else if(iter->detectorID <= detectorID_st2_max)
        {
            ++nHits_D2;
        }
        else
        {
            ++nHits_D3;
        }
    }
    int idx_D1 = nHits_D1;
    int idx_D2 = nHits_D1 + nHits_D2;
    int idx_D3 = nHits_D1 + nHits_D2 + nHits_D3;

    //Loop over all hits
    std::vector<Hit> hitTemp;
    hitTemp.assign(hitlist.begin(), hitlist.end());

    std::vector<int> flag(hitTemp.size(), -1);
    for(int i = idx_D2; i < idx_D3; ++i)
    {
        double z3 = p_geomSvc->getPlanePosition(hitTemp[i].detectorID);
        double slope_target = hitTemp[i].pos/(z3 - Z_TARGET);
        double slope_dump = hitTemp[i].pos/(z3 - Z_DUMP);
        for(int j = idx_D1; j < idx_D2; ++j)
        {
            if(p_geomSvc->getPlaneType(hitTemp[i].detectorID) != p_geomSvc->getPlaneType(hitTemp[j].detectorID)) continue;

            double z2 = p_geomSvc->getPlanePosition(hitTemp[j].detectorID);
            if(fabs((hitTemp[i].pos - hitTemp[j].pos)/(z2 - z3)) > TX_MAX) continue;
            double s2_target = hitTemp[j].pos - slope_target*(z2 - Z_TARGET);
            double s2_dump = hitTemp[j].pos - slope_dump*(z2 - Z_DUMP);

            for(int k = 0; k < idx_D1; ++k)
            {
                if(p_geomSvc->getPlaneType(hitTemp[i].detectorID) != p_geomSvc->getPlaneType(hitTemp[k].detectorID)) continue;
                if(flag[i] > 0 && flag[j] > 0 && flag[k] > 0) continue;

                double z1 = p_geomSvc->getPlanePosition(hitTemp[k].detectorID);
                double pos_exp_target = SAGITTA_TARGET_CENTER*s2_target + slope_target*(z1 - Z_TARGET);
                double pos_exp_dump = SAGITTA_DUMP_CENTER*s2_dump + slope_dump*(z1 - Z_DUMP);
                double win_target = fabs(s2_target*SAGITTA_TARGET_WIDTH);
                double win_dump = fabs(s2_dump*SAGITTA_DUMP_WIDTH);

                double p_min = std::min(pos_exp_target - win_target, pos_exp_dump - win_dump);
                double p_max = std::max(pos_exp_target + win_target, pos_exp_dump + win_dump);

                if(hitTemp[k].pos > p_min && hitTemp[k].pos < p_max)
                {
                    flag[i] = 1;
                    flag[j] = 1;
                    flag[k] = 1;
                }
            }
        }
    }

    int idx = 0;
    for(std::list<Hit>::iterator iter = hitlist.begin(); iter != hitlist.end(); )
    {
        if(flag[idx] < 0)
        {
            iter = hitlist.erase(iter);
        }
        else
        {
            ++iter;
        }

        ++idx;
        if(idx >= idx_D3) break;
    }
}

void EventReducer::deClusterize()
{
    std::vector<std::list<Hit>::iterator> cluster;
    cluster.clear();
    for(std::list<Hit>::iterator hit = hitlist.begin(); hit != hitlist.end(); ++hit)
    {
        //if we already reached the hodo part, stop
        if(hit->detectorID > nChamberPlanes) break;

        if(cluster.size() == 0)
        {
            cluster.push_back(hit);
        }
        else
        {
            if(hit->detectorID != cluster.back()->detectorID)
            {
                processCluster(cluster);
                cluster.push_back(hit);
            }
            else if(hit->elementID - cluster.back()->elementID > 1)
            {
                processCluster(cluster);
                cluster.push_back(hit);
            }
            else
            {
                cluster.push_back(hit);
            }
        }
    }
}

void EventReducer::processCluster(std::vector<std::list<Hit>::iterator>& cluster)
{
    unsigned int clusterSize = cluster.size();

    //size-2 clusters, retain the hit with smaller driftDistance
    if(clusterSize == 2)
    {
        double w_max = 0.9*0.5*(cluster.back()->pos - cluster.front()->pos);
        double w_min = w_max/9.*4.; //double w_min = 0.6*0.5*(cluster.back()->pos - cluster.front()->pos);

        if((cluster.front()->driftDistance > w_max && cluster.back()->driftDistance > w_min) || (cluster.front()->driftDistance > w_min && cluster.back()->driftDistance > w_max))
        {
            cluster.front()->driftDistance > cluster.back()->driftDistance ? hitlist.erase(cluster.front()) : hitlist.erase(cluster.back());
        }
        else if(fabs(cluster.front()->tdcTime - cluster.back()->tdcTime) < 8. && cluster.front()->detectorID >= 19 && cluster.front()->detectorID <= 24)
        {
            hitlist.erase(cluster.front());
            hitlist.erase(cluster.back());
        }
    }

    //size-larger-than-3, discard entirely
    if(clusterSize >= 3)
    {
        double dt_mean = 0.;
        for(unsigned int i = 1; i < clusterSize; ++i)
        {
            dt_mean += fabs(cluster[i]->tdcTime - cluster[i-1]->tdcTime);
        }
        dt_mean = dt_mean/(clusterSize - 1);

        if(dt_mean < 10.)
        {
            //electric noise, discard them all
            for(unsigned int i = 0; i < clusterSize; ++i)
            {
                hitlist.erase(cluster[i]);
            }
        }
        else
        {
            /*
            double dt_rms = 0.;
             	  for(unsigned int i = 1; i < clusterSize; ++i)
              {
                 double dt = fabs(cluster[i]->tdcTime - cluster[i-1]->tdcTime);
                 dt_rms += ((dt - dt_mean)*(dt - dt_mean));
              }
            dt_rms = sqrt(dt_rms/(clusterSize - 1));

            //delta ray, keep the first and last
            if(dt_rms < 5.)*/
            {
                for(unsigned int i = 1; i < clusterSize - 1; ++i)
                {
                    hitlist.erase(cluster[i]);
                }
            }
        }
    }

    cluster.clear();
}

void EventReducer::initHodoMaskLUT()
{
    h2celementID_lo.clear();
    h2celementID_hi.clear();

    TString hodoNames[8] = {"H1B", "H1T", "H2B", "H2T", "H3B", "H3T", "H4B", "H4T"};
    int hodoIDs[8];
    for(int i = 0; i < 8; ++i) hodoIDs[i] = p_geomSvc->getDetectorID(hodoNames[i].Data());

    int chamIDs[8][12] = { {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
                           {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12},
                           {13, 14, 15, 16, 17, 18, 0, 0, 0, 0, 0, 0},
                           {13, 14, 15, 16, 17, 18, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 25, 26, 27, 28, 29, 30},
                           {19, 20, 21, 22, 23, 24, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                           {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

    for(int i = 0; i < 8; ++i)
    {
        int nPaddles = p_geomSvc->getPlaneNElements(hodoIDs[i]);
        for(int j = 1; j <= nPaddles; ++j)
        {
            //for each paddle, there is a group of 6/12 chamber planes to work with
            int uniqueID = hodoIDs[i]*1000 + j;

            //get the four corners of the paddle
            double z0, x0_min, x0_max, y0_min, y0_max;
            z0 = p_geomSvc->getPlanePosition(hodoIDs[i]);
            p_geomSvc->get2DBoxSize(hodoIDs[i], j, x0_min, x0_max, y0_min, y0_max);

            for(int k = 0; k < 12; ++k)
            {
                if(chamIDs[i][k] < 1) continue;

                //project the box to the chamber plane with maximum xz/yz slope
                double z = p_geomSvc->getPlanePosition(chamIDs[i][k]);
                double x_min = x0_min - fabs(TX_MAX*(z - z0));
                double x_max = x0_max + fabs(TX_MAX*(z - z0));
                double y_min = y0_min - fabs(TY_MAX*(z - z0));
                double y_max = y0_max + fabs(TY_MAX*(z - z0));

                int elementID_lo = p_geomSvc->getPlaneNElements(chamIDs[i][k]);;
                int elementID_hi = 0;
                if(p_geomSvc->getPlaneType(chamIDs[i][k]) == 1)
                {
                    elementID_lo = p_geomSvc->getExpElementID(chamIDs[i][k], x_min);
                    elementID_hi = p_geomSvc->getExpElementID(chamIDs[i][k], x_max);
                }
                else
                {
                    for(int m = 1; m <= p_geomSvc->getPlaneNElements(chamIDs[i][k]); ++m)
                    {
                        double x1, y1, x2, y2;
                        p_geomSvc->getWireEndPoints(chamIDs[i][k], m, x1, x2, y1, y2);

                        if(!lineCrossing(x_min, y_min, x_min, y_max, x1, y1, x2, y2) &&
                           !lineCrossing(x_max, y_min, x_max, y_max, x1, y1, x2, y2)) continue;

                        if(m < elementID_lo) elementID_lo = m;
                        if(m > elementID_hi) elementID_hi = m;
                    }
                }

                elementID_lo -= 2;
                if(elementID_lo < 1) elementID_lo = 1;
                elementID_hi += 2;
                if(elementID_hi > p_geomSvc->getPlaneNElements(chamIDs[i][k])) elementID_hi = p_geomSvc->getPlaneNElements(chamIDs[i][k]);

                h2celementID_lo[uniqueID].push_back(chamIDs[i][k]*1000 + elementID_lo);
                h2celementID_hi[uniqueID].push_back(chamIDs[i][k]*1000 + elementID_hi);
            }
        }
    }

    //reverse the LUT
    c2helementIDs.clear();
    for(LUT::iterator iter = h2celementID_lo.begin(); iter != h2celementID_lo.end(); ++iter)
    {
        int hodoUID = iter->first;
        for(unsigned int i = 0; i < h2celementID_lo[hodoUID].size(); ++i)
        {
            int chamUID_lo = iter->second[i];
            int chamUID_hi = h2celementID_hi[hodoUID][i];
            for(int j = chamUID_lo; j <= chamUID_hi; ++j)
            {
                c2helementIDs[j].push_back(hodoUID);
            }
        }
    }
}

void EventReducer::hodoscopeMask(std::list<Hit>& chamberhits, std::list<Hit>& hodohits)
{
    for(std::list<Hit>::iterator iter = chamberhits.begin(); iter != chamberhits.end(); )
    {
        if(iter->detectorID > nChamberPlanes)
        {
            ++iter;
            continue;
        }

        int uniqueID = iter->uniqueID();

        bool masked = false;
        for(std::vector<int>::iterator jter = c2helementIDs[uniqueID].begin(); jter != c2helementIDs[uniqueID].end(); ++jter)
        {
            if(std::find(hodohits.begin(), hodohits.end(), Hit(*jter)) != hodohits.end())
            {
                masked = true;
                break;
            }
        }

        if(masked)
        {
            ++iter;
        }
        else
        {
            iter = chamberhits.erase(iter);
        }
    }
}

bool EventReducer::lineCrossing(double x1, double y1, double x2, double y2,
                                double x3, double y3, double x4, double y4)
{
    double tc = (x1 - x2)*(y3 - y1) + (y1 - y2)*(x1 - x3);
    double td = (x1 - x2)*(y4 - y1) + (y1 - y2)*(x1 - x4);

    return tc*td < 0;
}
