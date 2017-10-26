/*
GeomSvc.cxx

Implementation of class GeomSvc.

Author: Kun Liu, liuk@fnal.gov
Created: 10-19-2011
*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

#include <TROOT.h>
#include <TTree.h>
#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>
#include <TMath.h>
#include <TString.h>
#include <TPRegexp.h>
#include <TRotation.h>
#include <TMatrixD.h>

#include "GeomSvc.h"

Plane::Plane()
{
    detectorID = -1;
    planeType = -1;

    nElements = 0;
    x0 = 0.;
    y0 = 0.;
    z0 = 0.;

    x1 = 1.E6;
    y1 = 1.E6;
    x2 = -1.E6;
    y2 = -1.E6;

    thetaX = 0.;
    thetaY = 0.;
    thetaZ = 0.;
    rotX = 0.;
    rotY = 0.;
    rotZ = 0.;

    deltaX = 0.;
    deltaY = 0.;
    deltaZ = 0.;
    deltaW = 0.;
    for(int i = 0; i < 9; ++i) deltaW_module[i] = 0.;

    xc = 0.;
    yc = 0.;
    zc = 0.;
    wc = 0.;
    rX = 0.;
    rY = 0.;
    rZ = 0.;

    tmin = -1.E6;
    tmax = 1.E6;

    rtprofile = NULL;
}

void Plane::update()
{
    xc = x0 + deltaX;
    yc = y0 + deltaY;
    zc = z0 + deltaZ;

    rX = thetaX + rotX;
    rY = thetaY + rotY;
    rZ = thetaZ + rotZ;

    sintheta = sin(angleFromVert + rZ);
    costheta = cos(angleFromVert + rZ);
    tantheta = tan(angleFromVert + rZ);

    wc = getW(xc, yc);

    rotM[0][0] = cos(rZ)*cos(rY);
    rotM[0][1] = cos(rZ)*sin(rX)*sin(rY) - cos(rX)*sin(rZ);
    rotM[0][2] = cos(rX)*cos(rZ)*sin(rY) + sin(rX)*sin(rZ);
    rotM[1][0] = sin(rZ)*cos(rY);
    rotM[1][1] = sin(rZ)*sin(rX)*sin(rY) + cos(rZ)*cos(rX);
    rotM[1][2] = sin(rZ)*sin(rY)*cos(rX) - cos(rZ)*sin(rX);
    rotM[2][0] = -sin(rY);
    rotM[2][1] = cos(rY)*sin(rX);
    rotM[2][2] = cos(rY)*cos(rX);

    uVec[0] = cos(angleFromVert);
    uVec[1] = sin(angleFromVert);
    uVec[2] = 0.;

    vVec[0] = -sin(angleFromVert);
    vVec[1] = cos(angleFromVert);
    vVec[2] = 0.;

    nVec[0] = 0.;
    nVec[1] = 0.;
    nVec[2] = 1.;

    double temp[3];
    for(int i = 0; i < 3; ++i) temp[i] = uVec[i];
    for(int i = 0; i < 3; ++i)
    {
        uVec[i] = 0.;
        for(int j = 0; j < 3; ++j)
        {
            uVec[i] += rotM[i][j]*temp[j];
        }
    }

    for(int i = 0; i < 3; ++i) temp[i] = vVec[i];
    for(int i = 0; i < 3; ++i)
    {
        vVec[i] = 0.;
        for(int j = 0; j < 3; ++j)
        {
            vVec[i] += rotM[i][j]*temp[j];
        }
    }

    nVec[0] = uVec[1]*vVec[2] - vVec[1]*uVec[2];
    nVec[1] = uVec[2]*vVec[0] - vVec[2]*uVec[0];
    nVec[2] = uVec[0]*vVec[1] - vVec[0]*uVec[1];
}

double Plane::intercept(double tx, double ty, double x0_track, double y0_track) const
{
    //double mom[3] = {tx, ty, 1.};
    //double pos[3] = {x0_track, y0_track, 0.};

    double det = -(tx*nVec[0] + ty*nVec[1] + nVec[2]);
    double dpos[3] = {x0_track - xc, y0_track - yc, -zc};

    double vcp[3];
    vcp[0] = vVec[1] - vVec[2]*ty;
    vcp[1] = vVec[2]*tx - vVec[0];
    vcp[2] = vVec[0]*ty - vVec[1]*tx;

    //LogInfo(detectorID << "  " << detectorName << "  " << tx << "  " << ty << "  " << x0_track << "  " << y0_track << "  "  << -(vcp[0]*dpos[0] + vcp[1]*dpos[1] + vcp[2]*dpos[2])/det);
    return -(vcp[0]*dpos[0] + vcp[1]*dpos[1] + vcp[2]*dpos[2])/det + wc;
}

std::ostream& operator << (std::ostream& os, const Plane& plane)
{
    os << std::setw(6) << std::setiosflags(std::ios::right) << plane.detectorID
       << std::setw(6) << std::setiosflags(std::ios::right) << plane.detectorName
       << std::setw(6) << std::setiosflags(std::ios::right) << plane.planeType
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.spacing
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.xoffset
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.overlap
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.x2 - plane.x1
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.y2 - plane.y1
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.nElements
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.angleFromVert
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.z0
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.x0
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.y0
       << std::setw(10) << std::setiosflags(std::ios::right) << plane.deltaW;

    return os;
}

GeomSvc* GeomSvc::p_geometrySvc = NULL;

GeomSvc* GeomSvc::instance()
{
    if(p_geometrySvc == NULL)
    {
        p_geometrySvc = new GeomSvc;
        //p_geometrySvc->init();
    }

    return p_geometrySvc;
}

void GeomSvc::close()
{
    if(!p_geometrySvc)
    {
        for(int i = 0; i < nChamberPlanes+nHodoPlanes+nPropPlanes; ++i)
        {
            if(planes[i].rtprofile != NULL) delete planes[i].rtprofile;
        }

        delete p_geometrySvc;
    }
    else
    {
        std::cout << "Error: no instance of geometry service found! " << std::endl;
    }
}

void GeomSvc::init()
{
    using namespace std;

    //Initialize the detectorID --- detectorName convention
    typedef std::map<std::string, int>::value_type nameToID;

    map_detectorID.insert(nameToID("D0U", 1));
    map_detectorID.insert(nameToID("D0Up", 2));
    map_detectorID.insert(nameToID("D0X", 3));
    map_detectorID.insert(nameToID("D0Xp", 4));
    map_detectorID.insert(nameToID("D0V", 5));
    map_detectorID.insert(nameToID("D0Vp", 6));
    map_detectorID.insert(nameToID("D1V", 7));
    map_detectorID.insert(nameToID("D1Vp", 8));
    map_detectorID.insert(nameToID("D1X", 9));
    map_detectorID.insert(nameToID("D1Xp", 10));
    map_detectorID.insert(nameToID("D1U", 11));
    map_detectorID.insert(nameToID("D1Up", 12));
    map_detectorID.insert(nameToID("D2V", 13));
    map_detectorID.insert(nameToID("D2Vp", 14));
    map_detectorID.insert(nameToID("D2Xp", 15));
    map_detectorID.insert(nameToID("D2X", 16));
    map_detectorID.insert(nameToID("D2U", 17));
    map_detectorID.insert(nameToID("D2Up", 18));
    map_detectorID.insert(nameToID("D3pVp", 19));
    map_detectorID.insert(nameToID("D3pV", 20));
    map_detectorID.insert(nameToID("D3pXp", 21));
    map_detectorID.insert(nameToID("D3pX", 22));
    map_detectorID.insert(nameToID("D3pUp", 23));
    map_detectorID.insert(nameToID("D3pU", 24));
    map_detectorID.insert(nameToID("D3mVp", 25));
    map_detectorID.insert(nameToID("D3mV", 26));
    map_detectorID.insert(nameToID("D3mXp", 27));
    map_detectorID.insert(nameToID("D3mX", 28));
    map_detectorID.insert(nameToID("D3mUp", 29));
    map_detectorID.insert(nameToID("D3mU", 30));

    map_detectorID.insert(nameToID("H1B", nChamberPlanes+1));
    map_detectorID.insert(nameToID("H1T", nChamberPlanes+2));
    map_detectorID.insert(nameToID("H1L", nChamberPlanes+3));
    map_detectorID.insert(nameToID("H1R", nChamberPlanes+4));
    map_detectorID.insert(nameToID("H2L", nChamberPlanes+5));
    map_detectorID.insert(nameToID("H2R", nChamberPlanes+6));
    map_detectorID.insert(nameToID("H2B", nChamberPlanes+7));
    map_detectorID.insert(nameToID("H2T", nChamberPlanes+8));
    map_detectorID.insert(nameToID("H3B", nChamberPlanes+9));
    map_detectorID.insert(nameToID("H3T", nChamberPlanes+10));
    map_detectorID.insert(nameToID("H4Y1L", nChamberPlanes+11));
    map_detectorID.insert(nameToID("H4Y1R", nChamberPlanes+12));
    map_detectorID.insert(nameToID("H4Y2L", nChamberPlanes+13));
    map_detectorID.insert(nameToID("H4Y2R", nChamberPlanes+14));
    map_detectorID.insert(nameToID("H4B", nChamberPlanes+15));
    map_detectorID.insert(nameToID("H4T", nChamberPlanes+16));

    map_detectorID.insert(nameToID("P1Y1", nChamberPlanes+nHodoPlanes+1));
    map_detectorID.insert(nameToID("P1Y2", nChamberPlanes+nHodoPlanes+2));
    map_detectorID.insert(nameToID("P1X1", nChamberPlanes+nHodoPlanes+3));
    map_detectorID.insert(nameToID("P1X2", nChamberPlanes+nHodoPlanes+4));
    map_detectorID.insert(nameToID("P2X1", nChamberPlanes+nHodoPlanes+5));
    map_detectorID.insert(nameToID("P2X2", nChamberPlanes+nHodoPlanes+6));
    map_detectorID.insert(nameToID("P2Y1", nChamberPlanes+nHodoPlanes+7));
    map_detectorID.insert(nameToID("P2Y2", nChamberPlanes+nHodoPlanes+8));

    typedef std::map<int, std::string>::value_type idToName;
    for(std::map<std::string, int>::iterator iter = map_detectorID.begin(); iter != map_detectorID.end(); ++iter)
    {
        map_detectorName.insert(idToName(iter->second, iter->first));
    }

    ///Initialize the geometrical variables which should be from MySQL database
    //Connect server
    p_jobOptsSvc = JobOptsSvc::instance();
    TSQLServer* con = TSQLServer::Connect(p_jobOptsSvc->GetInputMySQLURL().c_str(), "seaguest","qqbar2mu+mu-");

    //Make query to Planes table
    char query[300];
    const char* buf_planes = "SELECT detectorName,spacing,cellWidth,overlap,numElements,angleFromVert,"
                             "xPrimeOffset,x0,y0,z0,planeWidth,planeHeight,theta_x,theta_y,theta_z from %s.Planes WHERE"
                             " detectorName LIKE 'D%%' OR detectorName LIKE 'H__' OR detectorName LIKE 'H____' OR "
                             "detectorName LIKE 'P____'";
    sprintf(query, buf_planes, p_jobOptsSvc->m_geomVersion.c_str());
    TSQLResult* res = con->Query(query);

    unsigned int nRows = res->GetRowCount();
    int dummy = 0;
    for(unsigned int i = 0; i < nRows; ++i)
    {
        TSQLRow* row = res->Next();
        string detectorName(row->GetField(0));
        toLocalDetectorName(detectorName, dummy);

        int detectorID = map_detectorID[detectorName];
        planes[detectorID].detectorID = detectorID;
        planes[detectorID].detectorName = detectorName;
        planes[detectorID].spacing = atof(row->GetField(1));
        planes[detectorID].cellWidth = atof(row->GetField(2));
        planes[detectorID].overlap = atof(row->GetField(3));
        planes[detectorID].angleFromVert = atof(row->GetField(5));
        planes[detectorID].xoffset = atof(row->GetField(6));
        planes[detectorID].thetaX = atof(row->GetField(12));
        planes[detectorID].thetaY = atof(row->GetField(13));
        planes[detectorID].thetaZ = atof(row->GetField(14));

        //Following items need to be sumed or averaged over all modules
        planes[detectorID].nElements += atoi(row->GetField(4));
        double x0_i = atof(row->GetField(7));
        double y0_i = atof(row->GetField(8));
        double z0_i = atof(row->GetField(9));
        double width_i = atof(row->GetField(10));
        double height_i = atof(row->GetField(11));

        double x1_i = x0_i - 0.5*width_i;
        double x2_i = x0_i + 0.5*width_i;
        double y1_i = y0_i - 0.5*height_i;
        double y2_i = y0_i + 0.5*height_i;

        planes[detectorID].x0 += x0_i;
        planes[detectorID].y0 += y0_i;
        planes[detectorID].z0 += z0_i;
        if(planes[detectorID].x1 > x1_i) planes[detectorID].x1 = x1_i;
        if(planes[detectorID].x2 < x2_i) planes[detectorID].x2 = x2_i;
        if(planes[detectorID].y1 > y1_i) planes[detectorID].y1 = y1_i;
        if(planes[detectorID].y2 < y2_i) planes[detectorID].y2 = y2_i;

        //Calculated value
        planes[detectorID].resolution = planes[detectorID].cellWidth/sqrt(12.);
        planes[detectorID].update();

        //Set the plane type
        if(detectorName.find("X") != string::npos || detectorName.find("T") != string::npos || detectorName.find("B") != string::npos)
        {
            planes[detectorID].planeType = 1;
        }
        else if((detectorName.find("U") != string::npos || detectorName.find("V") != string::npos) && planes[detectorID].angleFromVert > 0)
        {
            planes[detectorID].planeType = 2;
        }
        else if((detectorName.find("U") != string::npos || detectorName.find("V") != string::npos) && planes[detectorID].angleFromVert < 0)
        {
            planes[detectorID].planeType = 3;
        }
        else if(detectorName.find("Y") != string::npos || detectorName.find("L") != string::npos || detectorName.find("R") != string::npos)
        {
            planes[detectorID].planeType = 4;
        }

        delete row;
    }
    delete res;

    //For prop. tube only, average over 9 modules
    for(int i = nChamberPlanes+nHodoPlanes+1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i++)
    {
        planes[i].x0 = planes[i].x0/9.;
        planes[i].y0 = planes[i].y0/9.;
        planes[i].z0 = planes[i].z0/9.;

        planes[i].update();
    }
    cout << "GeomSvc: loaded basic spectrometer setup from geometry schema " << p_jobOptsSvc->m_geomVersion << endl;

    //load the initial value in the planeOffsets table
    if(p_jobOptsSvc->m_enableOnlineAlignment)
    {
        loadMilleAlignment(p_jobOptsSvc->m_alignmentFileMille);    //final chance of overwrite resolution numbers in online mode
        const char* buf_offsets = "SELECT detectorName,deltaX,deltaY,deltaZ,rotateAboutZ FROM %s.PlaneOffsets WHERE"
                                  " detectorName LIKE 'D%%' OR detectorName LIKE 'H__' OR detectorName LIKE 'H____' OR detectorName LIKE 'P____'";
        sprintf(query, buf_offsets, p_jobOptsSvc->m_geomVersion.c_str());
        res = con->Query(query);

        nRows = res->GetRowCount();
        if(nRows >= nChamberPlanes) cout << "GeomSvc: loaded chamber alignment parameters from database: " << p_jobOptsSvc->m_geomVersion.c_str() << endl;
        for(unsigned int i = 0; i < nRows; ++i)
        {
            TSQLRow* row = res->Next();
            string detectorName(row->GetField(0));
            toLocalDetectorName(detectorName, dummy);

            int detectorID = map_detectorID[detectorName];
            if(detectorID > nChamberPlanes) // ?? WTF?
            {
                delete row;
                continue;
            }

            planes[detectorID].deltaX = atof(row->GetField(1));
            planes[detectorID].deltaY = atof(row->GetField(2));
            planes[detectorID].deltaZ = atof(row->GetField(3));
            planes[detectorID].rotZ = atof(row->GetField(4));

            planes[detectorID].update();
            planes[detectorID].deltaW = planes[detectorID].getW(planes[detectorID].deltaX, planes[detectorID].deltaY);

            delete row;
        }
        delete res;
    }

    delete con;

    /////Here starts the user-defined part
    //load alignment parameters
    calibration_loaded = false;
    if(!p_jobOptsSvc->m_enableOnlineAlignment)
    {
        loadAlignment(p_jobOptsSvc->m_alignmentFileChamber, p_jobOptsSvc->m_alignmentFileHodo, p_jobOptsSvc->m_alignmentFileProp);
        loadMilleAlignment(p_jobOptsSvc->m_alignmentFileMille);
        loadCalibration(p_jobOptsSvc->m_calibrationsFile);
    }

    ///Initialize the position look up table for all wires, hodos, and tubes
    typedef std::map<std::pair<int, int>, double>::value_type posType;
    for(int i = 1; i <= nChamberPlanes; ++i)
    {
        for(int j = 1; j <= planes[i].nElements; ++j)
        {
            double pos = (j - (planes[i].nElements+1.)/2.)*planes[i].spacing + planes[i].xoffset + planes[i].x0*planes[i].costheta + planes[i].y0*planes[i].sintheta + planes[i].deltaW;
            map_wirePosition.insert(posType(make_pair(i, j), pos));
        }
    }

    // 2. for hodoscopes and prop. tubes
    for(int i = nChamberPlanes + 1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; ++i)
    {
        for(int j = 1; j <= planes[i].nElements; ++j)
        {
            double pos;
            if(i <= nChamberPlanes+nHodoPlanes)
            {
                pos = planes[i].x0*planes[i].costheta + planes[i].y0*planes[i].sintheta + planes[i].xoffset + (j - (planes[i].nElements+1)/2.)*planes[i].spacing + planes[i].deltaW;
            }
            else
            {
                int moduleID = 8 - int((j - 1)/8);        //Need to re-define moduleID for run2, note it's reversed compared to elementID
                pos = planes[i].x0*planes[i].costheta + planes[i].y0*planes[i].sintheta + planes[i].xoffset + (j - (planes[i].nElements+1)/2.)*planes[i].spacing + planes[i].deltaW_module[moduleID];
            }
            map_wirePosition.insert(posType(make_pair(i, j), pos));
        }
    }

    ///Initialize channel mapping  --- not needed at the moment
    xmin_kmag = -57.*2.54;
    xmax_kmag = 57.*2.54;
    ymin_kmag = -40.*2.54;
    ymax_kmag = 40.*2.54;

    zmin_kmag = 1064.26 - 120.*2.54;
    zmax_kmag = 1064.26 + 120.*2.54;

#ifdef _DEBUG_ON
    for(int i = 1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; ++i)
    {
        cout << planes[i] << endl;
    }
#endif
}

std::vector<int> GeomSvc::getDetectorIDs(std::string pattern)
{
    TPRegexp pattern_re(pattern.c_str());

    std::vector<int> detectorIDs;
    detectorIDs.clear();

    for(std::map<std::string, int>::iterator iter = map_detectorID.begin(); iter != map_detectorID.end(); ++iter)
    {
        TString detectorName((*iter).first.c_str());
        if(detectorName(pattern_re) != "")
        {
            detectorIDs.push_back((*iter).second);
        }
    }
    std::sort(detectorIDs.begin(), detectorIDs.end());

    return detectorIDs;
}

bool GeomSvc::findPatternInDetector(int detectorID, std::string pattern)
{
    TPRegexp pattern_re(pattern.c_str());
    TString detectorName(map_detectorName[detectorID]);

    return detectorName(pattern_re) != "";
}

bool GeomSvc::isInPlane(int detectorID, double x, double y)
{
    if(x < planes[detectorID].x1 || x > planes[detectorID].x2) return false;
    if(y < planes[detectorID].y1 || y > planes[detectorID].y2) return false;

    return true;
}

bool GeomSvc::isInElement(int detectorID, int elementID, double x, double y, double tolr)
{
    double x_min, x_max, y_min, y_max;
    get2DBoxSize(detectorID, elementID, x_min, x_max, y_min, y_max);

    x_min -= (tolr*(x_max - x_min));
    x_max += (tolr*(x_max - x_min));
    y_min -= (tolr*(y_max - y_min));
    y_max += (tolr*(y_max - y_min));

    return x > x_min && x < x_max && y > y_min && y < y_max;
}

bool GeomSvc::isInKMAG(double x, double y)
{
    if(x < xmin_kmag || x > xmax_kmag) return false;
    if(y < ymin_kmag || y > ymax_kmag) return false;

    return true;
}

void GeomSvc::getMeasurement(int detectorID, int elementID, double& measurement, double& dmeasurement)
{
    measurement = map_wirePosition[std::make_pair(detectorID, elementID)];
    dmeasurement = planes[detectorID].resolution;
}

double GeomSvc::getMeasurement(int detectorID, int elementID)
{
    return map_wirePosition[std::make_pair(detectorID, elementID)];
}

int GeomSvc::getExpElementID(int detectorID, double pos_exp)
{
    if(detectorID <= nChamberPlanes+nHodoPlanes)
    {
        int elementID_lo = int((pos_exp - planes[detectorID].xoffset - planes[detectorID].x0*planes[detectorID].costheta - planes[detectorID].y0*planes[detectorID].sintheta - planes[detectorID].deltaW + 0.5*(planes[detectorID].nElements + 1.)*planes[detectorID].spacing)/planes[detectorID].spacing);

        return fabs(pos_exp - map_wirePosition[std::make_pair(detectorID, elementID_lo)]) < 0.5*planes[detectorID].spacing ? elementID_lo : elementID_lo + 1;
    }

    int elementID = -1;
    for(int i = 1; i < planes[detectorID].nElements; i++)
    {
        double pos = map_wirePosition[std::make_pair(detectorID, i)];
        if(fabs(pos - pos_exp) < 0.5*planes[detectorID].cellWidth)
        {
            elementID = i;
            break;
        }
    }

    return elementID;
}

void GeomSvc::get2DBoxSize(int detectorID, int elementID, double& x_min, double& x_max, double& y_min, double& y_max)
{
    std::string detectorName = getDetectorName(detectorID);
    if(planes[detectorID].planeType == 1)
    {
        double x_center = map_wirePosition[std::make_pair(detectorID, elementID)];
        double x_width = 0.5*planes[detectorID].cellWidth;
        x_min = x_center - x_width;
        x_max = x_center + x_width;

        y_min = planes[detectorID].y1;
        y_max = planes[detectorID].y2;
    }
    else
    {
        double y_center = map_wirePosition[std::make_pair(detectorID, elementID)];
        double y_width = 0.5*planes[detectorID].cellWidth;
        y_min = y_center - y_width;
        y_max = y_center + y_width;

        x_min = planes[detectorID].x1;
        x_max = planes[detectorID].x2;
    }
}

void GeomSvc::getWireEndPoints(int detectorID, int elementID, double& x_min, double& x_max, double& y_min, double& y_max)
{
    y_min = planes[detectorID].y1;
    y_max = planes[detectorID].y2;

    double dw = planes[detectorID].xoffset + planes[detectorID].spacing*(elementID - (planes[detectorID].nElements + 1.)/2.);
    x_min = planes[detectorID].xc + dw*cos(planes[detectorID].rZ) - fabs(0.5*planes[detectorID].tantheta*(y_max - y_min));
    x_max = planes[detectorID].xc + dw*cos(planes[detectorID].rZ) + fabs(0.5*planes[detectorID].tantheta*(y_max - y_min));
}

void GeomSvc::toLocalDetectorName(std::string& detectorName, int& eID)
{
    using namespace std;

    if(detectorName.find("P") != string::npos)
    {
        string XY = detectorName[2] == 'H' ? "Y" : "X";
        string FB = (detectorName[3] == 'f' || detectorName[4] == 'f') ? "1" : "2"; //temporary solution
        int moduleID = std::isdigit(detectorName[3]) == 1 ? atoi(&detectorName[3]) : atoi(&detectorName[4]);

        detectorName.replace(2, detectorName.length(), "");
        detectorName += XY;
        detectorName += FB;

        if(eID <= 8) //Means either it's at lowest module or elementID is only from 1 to 8, repeatedly
        {
            eID = (9 - moduleID)*8 + eID;
        }
    }
    else if(detectorName.find("H4") != string::npos)
    {
        if(detectorName.find("T") != string::npos || detectorName.find("B") != string::npos)
        {
            detectorName.replace(3, detectorName.length(), "");
        }
        else
        {
            detectorName.replace(5, detectorName.length(), "");
        }
    }
}

double GeomSvc::getDriftDistance(int detectorID, double tdcTime)
{
    if(!calibration_loaded)
    {
        return 0.;
    }
    else if(planes[detectorID].rtprofile == NULL)
    {
        return 0.;
    }
    else if(tdcTime < planes[detectorID].tmin)
    {
        return 0.5*planes[detectorID].cellWidth;
    }
    else if(tdcTime > planes[detectorID].tmax)
    {
        return 0.;
    }
    else
    {
        return planes[detectorID].rtprofile->Eval(tdcTime);
    }

    return 0.;
}

double GeomSvc::getInterceptionFast(int detectorID, double tx, double ty, double x0, double y0) const
{
    return (tx*planes[detectorID].zc + x0)*planes[detectorID].costheta + (ty*planes[detectorID].zc + y0)*planes[detectorID].sintheta;
}

void GeomSvc::loadAlignment(const std::string& alignmentFile_chamber, const std::string& alignmentFile_hodo, const std::string& alignmentFile_prop)
{
    using namespace std;

    //load alignment numbers for chambers
    fstream _align_chamber;
    _align_chamber.open(alignmentFile_chamber.c_str(), ios::in);

    char buf[300];
    if(_align_chamber)
    {
        for(int i = 1; i <= nChamberPlanes; i++)
        {
            _align_chamber.getline(buf, 100);
            istringstream stringBuf(buf);

            stringBuf >> planes[i].deltaW >> planes[i].resolution;
            if(planes[i].resolution < RESOLUTION_DC) planes[i].resolution = RESOLUTION_DC;

            planes[i].deltaX = planes[i].deltaW*planes[i].costheta;
            planes[i].deltaY = planes[i].deltaW*planes[i].sintheta;
            planes[i].update();
        }

        for(int i = 1; i <= nChamberPlanes; i += 2)
        {
            double resol = planes[i].resolution > planes[i+1].resolution ? planes[i].resolution : planes[i+1].resolution;
            planes[i].resolution = resol;
            planes[i].resolution = resol;
        }

        cout << "GeomSvc: loaded chamber alignment parameters from " << alignmentFile_chamber << endl;
    }
    _align_chamber.close();

    //load alignment numbers for hodos
    fstream _align_hodo;
    _align_hodo.open(alignmentFile_hodo.c_str(), ios::in);

    if(_align_hodo)
    {
        for(int i = nChamberPlanes+1; i <= nChamberPlanes+nHodoPlanes; i++)
        {
            _align_hodo.getline(buf, 100);
            istringstream stringBuf(buf);

            stringBuf >> planes[i].deltaW;
            planes[i].deltaX = planes[i].deltaW*planes[i].costheta;
            planes[i].deltaY = planes[i].deltaW*planes[i].sintheta;
            planes[i].update();
        }
        cout << "GeomSvc: loaded hodoscope alignment parameters from " << alignmentFile_hodo << endl;
    }
    else
    {
        cout << "GeomSvc: failed to load hodoscope alignment parameters from " << alignmentFile_hodo << endl;
    }
    _align_hodo.close();

    //load alignment numbers for prop. tubes
    fstream _align_prop;
    _align_prop.open(alignmentFile_prop.c_str(), ios::in);

    if(_align_prop)
    {
        for(int i = nChamberPlanes+nHodoPlanes+1; i <= nChamberPlanes+nHodoPlanes+nPropPlanes; i += 2)
        {
            planes[i].deltaW = 0.;
            planes[i+1].deltaW = 0.;
            for(int j = 0; j < 9; j++)
            {
                _align_prop.getline(buf, 100);
                istringstream stringBuf(buf);

                stringBuf >> planes[i].deltaW_module[j];
                planes[i+1].deltaW_module[j] = planes[i].deltaW_module[j];

                planes[i].deltaW += planes[i].deltaW_module[j];
                planes[i+1].deltaW += planes[i+1].deltaW_module[j];
            }

            planes[i].deltaW /= 9.;
            planes[i].deltaX = planes[i].deltaW*planes[i].costheta;
            planes[i].deltaY = planes[i].deltaW*planes[i].sintheta;

            planes[i+1].deltaW /= 9.;
            planes[i+1].deltaX = planes[i+1].deltaW*planes[i+1].costheta;
            planes[i+1].deltaY = planes[i+1].deltaW*planes[i+1].sintheta;
        }
        cout << "GeomSvc: loaded prop. tube alignment parameters from " << alignmentFile_prop << endl;
    }
    else
    {
        cout << "GeomSvc: failed to load prop. tube alignment parameters from " << alignmentFile_prop << endl;
    }

}

void GeomSvc::loadMilleAlignment(const std::string& alignmentFile_mille)
{
    using namespace std;

    //load alignment numbers for chambers
    fstream _align_mille;
    _align_mille.open(alignmentFile_mille.c_str(), ios::in);

    char buf[300];
    if(_align_mille)
    {
        for(int i = 1; i <= nChamberPlanes; i++)
        {
            _align_mille.getline(buf, 100);
            istringstream stringBuf(buf);

            stringBuf >> planes[i].deltaZ >> planes[i].rotZ >> planes[i].deltaW >> planes[i].resolution;
            planes[i].deltaX = planes[i].deltaW*planes[i].costheta;
            planes[i].deltaY = planes[i].deltaW*planes[i].sintheta;
            planes[i].update();

            //if(planes[i].resolution < RESOLUTION_DC) planes[i].resolution = RESOLUTION_DC;
        }
        cout << "GeomSvc: loaded millepede-based alignment parameters from " << alignmentFile_mille << endl;

        for(int i = 1; i <= nChamberPlanes; i+=2)
        {
            planes[i].resolution = RESOLUTION_DC*0.5*(planes[i].resolution + planes[i+1].resolution);
            planes[i+1].resolution = planes[i].resolution;
        }
    }
    else
    {
        cout << "GeomSvc: failed to load mullepede-based alignment parameters from " << alignmentFile_mille << endl;
    }

    _align_mille.close();
}

void GeomSvc::loadCalibration(const std::string& calibrationFile)
{
    using namespace std;

    fstream _cali_file;
    _cali_file.open(calibrationFile.c_str(), ios::in);

    char buf[300];
    int iBin, nBin, detectorID;
    double tmin_temp, tmax_temp;
    double R[500], T[500];
    if(_cali_file)
    {
        calibration_loaded = true;

        while(_cali_file.getline(buf, 100))
        {
            istringstream detector_info(buf);
            detector_info >> detectorID >> nBin >> tmin_temp >> tmax_temp;
            planes[detectorID].tmax = tmax_temp;
            planes[detectorID].tmin = tmin_temp;

            for(int i = 0; i < nBin; i++)
            {
                _cali_file.getline(buf, 100);
                istringstream cali_line(buf);

                cali_line >> iBin >> T[i] >> R[i];
            }

            if(planes[detectorID].rtprofile != NULL) delete planes[detectorID].rtprofile;
            if(nBin > 0) planes[detectorID].rtprofile = new TSpline3(getDetectorName(detectorID).c_str(), T, R, nBin, "b1e1");
        }
        cout << "GeomSvc: loaded calibration parameters from " << calibrationFile << endl;
    }
    _cali_file.close();
}

bool GeomSvc::isInTime(int detectorID, double tdcTime)
{
    return tdcTime > planes[detectorID].tmin && tdcTime < planes[detectorID].tmax;
}

void GeomSvc::printWirePosition()
{
    for(std::map<std::string, int>::iterator iter = map_detectorID.begin(); iter != map_detectorID.end(); ++iter)
    {
        int detectorID = (*iter).second;
        std::cout << " ====================== " << (*iter).first << " ==================== " << std::endl;
        for(int i = 1; i <= planes[detectorID].nElements; ++i)
        {
            std::cout << std::setw(6) << std::setiosflags(std::ios::right) << detectorID;
            std::cout << std::setw(6) << std::setiosflags(std::ios::right) << (*iter).first;
            std::cout << std::setw(6) << std::setiosflags(std::ios::right) << i;
            std::cout << std::setw(10) << std::setiosflags(std::ios::right) << map_wirePosition[std::make_pair(detectorID, i)];
            std::cout << std::setw(10) << std::setiosflags(std::ios::right) << map_wirePosition[std::make_pair(detectorID, i)] - 0.5*planes[detectorID].cellWidth;
            std::cout << std::setw(10) << std::setiosflags(std::ios::right) << map_wirePosition[std::make_pair(detectorID, i)] + 0.5*planes[detectorID].cellWidth;
            std::cout << std::endl;
        }
    }
}

void GeomSvc::printAlignPar()
{
    std::cout << "detectorID         DetectorName            offset_pos             offset_z             offset_phi" << std::endl;
    for(std::map<std::string, int>::iterator iter = map_detectorID.begin(); iter != map_detectorID.end(); ++iter)
    {
        std::cout << iter->second << "     " << iter->first << "    " << planes[(*iter).second].deltaW << "     " << planes[(*iter).second].deltaZ << "      " << planes[(*iter).second].rotZ << std::endl;
    }
}

void GeomSvc::printTable()
{
    std::cout << "detectorName    detectorID  planeType    Spacing     Xoffset     overlap     width       height       nElement       angleFromVertical       Z" << std::endl;
    for(std::map<std::string, int>::iterator iter = map_detectorID.begin(); iter != map_detectorID.end(); ++iter)
    {
        std::cout << planes[iter->second] << std::endl;
    }
}
