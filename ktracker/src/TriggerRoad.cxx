#include <iomanip>
#include <stdlib.h>
#include <algorithm>

#include "TriggerRoad.h"
#include "GeomSvc.h"

ClassImp(TriggerRoad)

TriggerRoad::TriggerRoad()
{
    targetWeight = 0.;
    dumpWeight = 0.;
    lowMWeight = 0.;
    highMWeight = 0.;

    detectorIDs.clear();
    elementIDs.clear();

    px_min = 0.;
    px_max = 0.;
    px_mean = 0.;

    rndf = 0.;

    nPlus = 0;
    nMinus = 0;

    enabled = true;
}

TriggerRoad::TriggerRoad(std::list<int> uniqueIDs)
{
    for(std::list<int>::iterator iter = uniqueIDs.begin(); iter != uniqueIDs.end(); ++iter)
    {
        if(*iter < 0) continue;

        int elementID = *iter % 100;
        int detectorID = (*iter - elementID)/100;

        addElement(detectorID, elementID);
    }
}

TriggerRoad::TriggerRoad(Tracklet& tracklet) : detectorIDs(4), elementIDs(4)
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    const int hodoIDs[2][4] = {{32, 38, 40, 46}, {31, 37, 39, 45}};

    int tb = 0;
    for(int i = 0; i < 4; ++i)
    {
        tb += (p_geomSvc->isInPlane(hodoIDs[0][i], 0., tracklet.getExpPositionY(p_geomSvc->getPlanePosition(hodoIDs[0][i]))) ? 1 : -1);
        //std::cout << i << " " << tb << " " << p_geomSvc->getPlanePosition(hodoIDs[0][i]) << " " << tracklet.getExpPositionY(p_geomSvc->getPlanePosition(hodoIDs[0][i])) << std::endl;
    }

    if(tb == 0)
    {
        detectorIDs.clear();
        return;
    }

    tb = tb > 0 ? 0 : 1;
    for(int i = 0; i < 4; ++i)
    {
        detectorIDs[i] = hodoIDs[tb][i];
        elementIDs[i] = p_geomSvc->getExpElementID(hodoIDs[tb][i], tracklet.getExpPositionX(p_geomSvc->getPlanePosition(hodoIDs[tb][i])));
        //std::cout << i << " " << detectorIDs[i] << " " << elementIDs[i] << std::endl;
    }
}

TriggerRoad::TriggerRoad(SRecTrack& track) : detectorIDs(4), elementIDs(4)
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    const int hodoIDs[2][4] = {{32, 38, 40, 46}, {31, 37, 39, 45}};

    double x_exp[4], y_exp[4];
    for(int i = 0; i < 4; ++i)
    {
        track.getExpPositionFast(p_geomSvc->getPlanePosition(hodoIDs[0][i]), x_exp[i], y_exp[i]);
    }

    int tb = 0;
    for(int i = 0; i < 4; ++i)
    {
        tb += (p_geomSvc->isInPlane(hodoIDs[0][i], 0., y_exp[i]) ? 1 : -1);
    }

    if(tb == 0)
    {
        detectorIDs.clear();
        return;
    }

    tb = tb > 0 ? 0 : 1;
    for(int i = 0; i < 4; ++i)
    {
        detectorIDs[i] = hodoIDs[tb][i];
        elementIDs[i] = p_geomSvc->getExpElementID(hodoIDs[tb][i], x_exp[i]);
        //std::cout << i << " " << detectorIDs[i] << " " << elementIDs[i] << std::endl;
    }
}

bool TriggerRoad::isValid()
{
    if(!enabled) return false;
    if(detectorIDs.size() != 4) return false;
    if(getTB() == 0) return false;

    //Add whatever here

    return true;
}

void TriggerRoad::print()
{
    std::cout << "For this road: " << ratio() << std::endl;
    for(int i = 0; i < 4; i++)
    {
        std::cout << detectorIDs[i] << "   " << elementIDs[i] << " : ";
    }
    std::cout << std::endl;
}

std::ostream& operator << (std::ostream& os, const TriggerRoad& road)
{
    //os << std::setw(6) << road.detectorIDs[0] << std::setw(6) << road.detectorIDs[1] << std::setw(6) << road.detectorIDs[2] << std::setw(6) << road.detectorIDs[3]
    os << std::setw(6) << road.elementIDs[0] << std::setw(6) << road.elementIDs[1] << std::setw(6) << road.elementIDs[2] << std::setw(6) << road.elementIDs[3]
       << std::setprecision(3) << std::setw(10) << std::setiosflags(std::ios::right) << road.px_mean
       << std::setprecision(3) << std::setw(10) << std::setiosflags(std::ios::right) << road.getpXWidth()
       << std::setprecision(3) << std::setw(10) << std::setiosflags(std::ios::right) << road.px_min
       << std::setprecision(3) << std::setw(10) << std::setiosflags(std::ios::right) << road.px_max
       << std::setw(6) << std::setiosflags(std::ios::right) << abs(road.groupID)
       << std::setw(10) << road.getNEntries()
       << std::setprecision(4) << std::setw(20) << std::setiosflags(std::ios::right) << road.weight()
       << std::setprecision(4) << std::setw(20) << std::setiosflags(std::ios::right) << road.ratio()
       //<< std::setprecision(4) << std::setw(20) << std::setiosflags(std::ios::right) << road.nPlus
       //<< std::setprecision(4) << std::setw(20) << std::setiosflags(std::ios::right) << road.nMinus
       << std::setprecision(5) << std::setw(20) << std::setiosflags(std::ios::right) << road.rndf;

    return os;
}

void TriggerRoad::clear()
{
    roadID = -1;
    targetWeight = 0.;
    dumpWeight = 0.;

    detectorIDs.clear();
    elementIDs.clear();

    pXs.clear();

    enabled = true;
}

void TriggerRoad::addElement(int detectorID, int elementID)
{
    if(std::find(detectorIDs.begin(), detectorIDs.end(), detectorID) != detectorIDs.end()) return;

    detectorIDs.push_back(detectorID);
    elementIDs.push_back(elementID);
}

bool TriggerRoad::operator==(const TriggerRoad& elem) const
{
    if(detectorIDs.size() != elem.detectorIDs.size()) return false;

    int nElements = detectorIDs.size();
    for(int i = 0; i < nElements; i++)
    {
        if(detectorIDs[i] != elem.detectorIDs[i]) return false;
        if(elementIDs[i] != elem.elementIDs[i]) return false;
    }

    return true;
}

bool TriggerRoad::byTargetDump(const TriggerRoad& elem1, const TriggerRoad& elem2)
{
    return elem1.ratio() > elem2.ratio();
}

bool TriggerRoad::byWeight(const TriggerRoad& elem1, const TriggerRoad& elem2)
{
    return elem1.weight() > elem2.weight();
}

bool TriggerRoad::byMass(const TriggerRoad& elem1, const TriggerRoad& elem2)
{
    return elem1.mratio() > elem2.mratio();
}

bool TriggerRoad::byPt(const TriggerRoad& elem1, const TriggerRoad& elem2)
{
    return elem1.px_min > elem2.px_min;
}

bool TriggerRoad::byRndFrequency(const TriggerRoad& elem1, const TriggerRoad& elem2)
{
    return elem1.rndf > elem2.rndf;
}

TriggerRoad& TriggerRoad::operator+=(const TriggerRoad& elem)
{
    targetWeight += elem.targetWeight;
    dumpWeight += elem.dumpWeight;
    lowMWeight += elem.lowMWeight;
    highMWeight += elem.highMWeight;

    px_mean = (px_mean*pXs.size() + elem.px_mean*elem.pXs.size())/(pXs.size() + elem.pXs.size());
    px_min = px_min < elem.px_min ? px_min : elem.px_min;
    px_max = px_max > elem.px_max ? px_max : elem.px_max;
    pXs.insert(pXs.end(), elem.pXs.begin(), elem.pXs.end());

    nPlus += elem.nPlus;
    nMinus += elem.nMinus;

    return *this;
}

double TriggerRoad::getpXMean() const
{
    double sum = 0.;
    for(unsigned int i = 0; i < pXs.size(); i++)
    {
        sum += pXs[i];
    }

    return sum/pXs.size();
}

int TriggerRoad::getRoadID()
{
    if(detectorIDs.size() != 4) return 0;

    return getTB()*((elementIDs[0]-1)*16*16*16 + (elementIDs[1]-1)*16*16 + (elementIDs[2]-1)*16 + elementIDs[3]);
}

double TriggerRoad::getpXWidth() const
{
    double sigma = 0.;
    for(unsigned int i = 0; i < pXs.size(); i++)
    {
        sigma += ((pXs[i] - px_mean)*(pXs[i] - px_mean));
    }

    return sqrt(sigma/pXs.size());
}

int TriggerRoad::getLR()
{
    GeomSvc* p_geomSvc = GeomSvc::instance();

    int LR = 0;
    for(int i = 1; i < 4; i++)
    {
        if(elementIDs[i] < p_geomSvc->getPlaneNElements(detectorIDs[i])/2)
        {
            LR += 1;
        }
        else
        {
            LR += -1;
        }
    }

    int LR1 = 0;
    LR1 = elementIDs[0] < p_geomSvc->getPlaneNElements(detectorIDs[0])/2 ? 1 : -1;

    if(LR*LR1 == 3) return 1;
    if(LR*LR1 == -3) return -1;
    return 0;
}

int TriggerRoad::getTB()
{
    int TB = 0;
    for(int i = 0; i < 4; i++)
    {
        if((detectorIDs[i] & 1) == 0)
        {
            TB += 1;
        }
        else
        {
            TB += -1;
        }
    }

    if(TB == 4) return 1;
    if(TB == -4) return -1;
    return 0;
}

TriggerRoad TriggerRoad::reflectTB()
{
    TriggerRoad reflected = *this;

    int corr = getTB();
    for(int i = 0; i < 4; ++i)
    {
        reflected.detectorIDs[i] -= corr;
    }

    return reflected;
}

TriggerRoad TriggerRoad::reflectLR()
{
    GeomSvc* p_geomSvc = GeomSvc::instance();
    TriggerRoad reflected = *this;

    for(int i = 0; i < 4; ++i)
    {
        int nElements = p_geomSvc->getPlaneNElements(detectorIDs[i]);
        reflected.elementIDs[i] = nElements - reflected.elementIDs[i] + 1;
    }

    return reflected;
}

std::list<TriggerRoad> TriggerRoad::makeRoadList(int nHits, int dIDs[], int eIDs[], double z, double mass, double pX, double weight)
{
    GeomSvc* p_geomSvc = GeomSvc::instance();

    std::list<TriggerRoad> roads_new;
    std::vector<std::pair<int, int> > hodoHits[4];
    for(int i = 0; i < nHits; i++)
    {
        std::string detectorName = p_geomSvc->getDetectorName(dIDs[i]);
        if(detectorName.find("H1T") != std::string::npos || detectorName.find("H1B") != std::string::npos)
        {
            hodoHits[0].push_back(std::make_pair(dIDs[i], eIDs[i]));
        }
        else if(detectorName.find("H2T") != std::string::npos || detectorName.find("H2B") != std::string::npos)
        {
            hodoHits[1].push_back(std::make_pair(dIDs[i], eIDs[i]));
        }
        else if(detectorName.find("H3T") != std::string::npos || detectorName.find("H3B") != std::string::npos)
        {
            hodoHits[2].push_back(std::make_pair(dIDs[i], eIDs[i]));
        }
        else if(detectorName.find("H4T") != std::string::npos || detectorName.find("H4B") != std::string::npos)
        {
            hodoHits[3].push_back(std::make_pair(dIDs[i], eIDs[i]));
        }
    }

    for(std::vector<std::pair<int, int> >::iterator iter = hodoHits[0].begin(); iter != hodoHits[0].end(); ++iter)
    {
        for(std::vector<std::pair<int, int> >::iterator jter = hodoHits[1].begin(); jter != hodoHits[1].end(); ++jter)
        {
            for(std::vector<std::pair<int, int> >::iterator kter = hodoHits[2].begin(); kter != hodoHits[2].end(); ++kter)
            {
                for(std::vector<std::pair<int, int> >::iterator lter = hodoHits[3].begin(); lter != hodoHits[3].end(); ++lter)
                {
                    TriggerRoad road_new;
                    road_new.addElement(iter->first, iter->second);
                    road_new.addElement(jter->first, jter->second);
                    road_new.addElement(kter->first, kter->second);
                    road_new.addElement(lter->first, lter->second);
                    road_new.enable();

                    if(!road_new.isValid()) continue;

                    pX = fabs(pX);
                    road_new.pXs.push_back(pX);
                    road_new.px_mean = pX;
                    road_new.px_min = pX;
                    road_new.px_max = pX;
                    if(z > 0)
                    {
                        road_new.dumpWeight = weight;
                    }
                    else
                    {
                        road_new.targetWeight = weight;
                    }

                    if(mass < 4.)
                    {
                        road_new.lowMWeight = weight;
                    }
                    else
                    {
                        road_new.highMWeight = weight;
                    }

                    road_new.px_mean = pX;
                    roads_new.push_back(road_new);
                    roads_new.push_back(road_new.reflectTB());
                }
            }
        }
    }

    //if(roads_new.size() > 1) std::cout << "!!!!!!!!!!!!!!!!   " << roads_new.size() << std::endl;
    return roads_new;
}
