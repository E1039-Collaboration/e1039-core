/*
GeomSvc.h

Definition of the class GeomSvc, which provide geometry service to the
whole tracking system.

For now the drift time is not used so as to avoid left-right ambiguity

Author: Kun Liu, liuk@fnal.gov
Created: 10-18-2011

Updated by Kun Liu on 07-03-2012
---  Support all detector planes including chambers, hodoscopes, and prop.
tubes
---  The convention of detector plane ID is: firstly chambers, then hodos,
and then prop. tubes
*/

#ifndef _GEOMSVC_H
#define _GEOMSVC_H

#include "GlobalConsts.h"

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <TVector3.h>
#include <TSpline.h>

#include "JobOptsSvc.h"

class Plane
{
public:
    //Default constructor
    Plane();

    //Get interception with track
    double intercept(double tx, double ty, double x0_track, double y0_track) const;

    //X, Y, U, V conversion
    double getX(double w, double y) const { return w/costheta - y*tantheta; }
    double getY(double x, double w) const { return w/sintheta - x/tantheta; }
    double getW(double x, double y) const { return x*costheta + y*sintheta; }

    //Calculate the internal variables
    void update();

    //Debugging output
    friend std::ostream& operator << (std::ostream& os, const Plane& plane);

public:
    //Detector identifier
    int detectorID;
    std::string detectorName;
    int planeType;   //X = 1, angleFromVert > 0 = 2, angleFromVert < 0 = 3, Y = 4

    //Ideal properties
    int nElements;
    double spacing;
    double cellWidth;
    double xoffset;
    double overlap;
    double angleFromVert;
    double sintheta;
    double costheta;
    double tantheta;

    //Survey info
    double x0;     //x0, y0, z0 define the center of detector
    double y0;
    double z0;
    double x1;     //x1, y1 define the lower/left edge of detector
    double y1;
    double x2;     //x2, y2 define the upper/right edge of detector
    double y2;
    double thetaX;
    double thetaY;
    double thetaZ;

    //Alignment info
    double deltaX;
    double deltaY;
    double deltaZ;
    double deltaW;             //for chambers and hodos
    double deltaW_module[9];   //for prop. tubes only
    double rotX;
    double rotY;
    double rotZ;
    double resolution;

    //Final position/rotation
    double xc;
    double yc;
    double zc;
    double wc;
    double rX;
    double rY;
    double rZ;

    //Geometric setup
    double nVec[3];             //Perpendicular to plane
    double uVec[3];             //measuring direction
    double vVec[3];             //non-measuring direction
    double rotM[3][3];          //rotation matrix

    //Calibration info
    double tmin;
    double tmax;
    TSpline3* rtprofile;
};

class GeomSvc
{
public:
    ///singlton instance
    static GeomSvc* instance();

    ///Initialization, either from MySQL or from ascii file
    void init();
    void loadCalibration(const std::string& calibrateFile);
    void loadAlignment(const std::string& alignmentFile_chamber, const std::string& alignmentFile_hodo, const std::string& alignmentFile_prop);
    void loadMilleAlignment(const std::string& alignmentFile_mille);

    ///Close the geometry service before exit or starting a new one
    void close();

    ///Convert the official detectorName to local detectorName
    void toLocalDetectorName(std::string& detectorName, int& eID);

    ///Get the plane position
    int getDetectorID(std::string detectorName) { return map_detectorID[detectorName]; }
    std::string getDetectorName(int detectorID) { return map_detectorName[detectorID]; }
    std::vector<int> getDetectorIDs(std::string pattern);
    bool findPatternInDetector(int detectorID, std::string pattern);

    Plane getPlane(int detectorID) const { return planes[detectorID]; }
    double getPlanePosition(int detectorID) const { return planes[detectorID].zc; }
    double getPlaneSpacing(int detectorID) const  { return planes[detectorID].spacing; }
    double getCellWidth(int detectorID)     { return planes[detectorID].cellWidth; }
    double getCostheta(int detectorID) const  { return planes[detectorID].costheta; }
    double getSintheta(int detectorID) const  { return planes[detectorID].sintheta; }
    double getTantheta(int detectorID) const  { return planes[detectorID].tantheta; }
    double getPlaneScaleX(int detectorID)   { return planes[detectorID].x2 - planes[detectorID].x1; }
    double getPlaneScaleY(int detectorID)   { return planes[detectorID].y2 - planes[detectorID].y1; }
    int getPlaneNElements(int detectorID)   { return planes[detectorID].nElements; }
    double getPlaneResolution(int detectorID) const { return planes[detectorID].resolution; }

    double getPlaneCenterX(int detectorID)  { return planes[detectorID].xc; }
    double getPlaneCenterY(int detectorID)  { return planes[detectorID].yc; }
    double getPlaneCenterZ(int detectorID)  { return planes[detectorID].zc; }
    double getRotationInX(int detectorID)    { return planes[detectorID].rX; }
    double getRotationInY(int detectorID)    { return planes[detectorID].rY; }
    double getRotationInZ(int detectorID)    { return planes[detectorID].rZ; }

    double getPlaneZOffset(int detectorID)   { return planes[detectorID].deltaZ; }
    double getPlanePhiOffset(int detectorID) { return planes[detectorID].rotZ; }
    double getPlaneWOffset(int detectorID)   { return planes[detectorID].deltaW; }
    double getPlaneWOffset(int detectorID, int moduleID) { return planes[detectorID].deltaW_module[moduleID]; }

    int getPlaneType(int detectorID) const { return planes[detectorID].planeType; }

    double getKMAGCenter()     { return (zmin_kmag + zmax_kmag)/2.; }
    double getKMAGUpstream()   { return zmin_kmag; }
    double getKMAGDownstream() { return zmax_kmag; }

    ///Get the interception of a line an a plane
    double getInterception(int detectorID, double tx, double ty, double x0, double y0) const { return planes[detectorID].intercept(tx, ty, x0, y0); }
    double getInterceptionFast(int detectorID, double tx, double ty, double x0, double y0) const;
    double getInterceptionFast(int detectorID, double x_exp, double y_exp) const { return planes[detectorID].getW(x_exp, y_exp); }

    ///Convert the detectorID and elementID to the actual hit position
    void getMeasurement(int detectorID, int elementID, double& measurement, double& dmeasurement);
    double getMeasurement(int detectorID, int elementID);
    void get2DBoxSize(int detectorID, int elementID, double& x_min, double& x_max, double& y_min, double& y_max);
    void getWireEndPoints(int detectorID, int elementID, double& x_min, double& x_max, double& y_min, double& y_max);
    int getExpElementID(int detectorID, double pos_exp);

    ///Calibration related
    bool isCalibrationLoaded() { return calibration_loaded; }
    double getDriftDistance(int detectorID, double tdcTime);
    bool isInTime(int detectorID, double tdcTime);
    TSpline3* getRTCurve(int detectorID) { return planes[detectorID].rtprofile; }

    ///Convert the stereo hits to Y value
    double getYinStereoPlane(int detectorID, double x, double u) { return planes[detectorID].getY(x, u); }
    double getUinStereoPlane(int detectorID, double x, double y) { return planes[detectorID].getW(x, y); }
    double getXinStereoPlane(int detectorID, double u, double y) { return planes[detectorID].getX(u, y); }

    ///See if a point is in a plane
    bool isInPlane(int detectorID, double x, double y);
    bool isInElement(int detectorID, int elementID, double x, double y, double tolr = 0.);
    bool isInKMAG(double x, double y);

    ///Debugging print of the content
    void printAlignPar();
    void printTable();
    void printWirePosition();

private:

    //All the detector planes
    Plane planes[nChamberPlanes+nHodoPlanes+nPropPlanes+1];

    //flag of loading calibration parameters
    bool calibration_loaded;

    //Position of KMag
    double xmin_kmag, xmax_kmag;
    double ymin_kmag, ymax_kmag;
    double zmin_kmag, zmax_kmag;

    //Mapping of detectorName to detectorID, and vice versa
    std::map<std::string, int> map_detectorID;
    std::map<int, std::string> map_detectorName;

    //Mapping to wire position
    std::map<std::pair<int, int>, double> map_wirePosition;

    //Pointer to job option service
    JobOptsSvc* p_jobOptsSvc;

    //singleton pointor
    static GeomSvc* p_geometrySvc;
};

#endif
