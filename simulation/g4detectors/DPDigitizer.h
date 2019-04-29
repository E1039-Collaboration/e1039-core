#ifndef DPDigitizer_H
#define DPDigitizer_H

#include <fun4all/SubsysReco.h>

#include <vector>
#include <string>
#include <map>

#ifndef __CINT__
#include <boost/array.hpp>
#include <Geant4/G4ThreeVector.hh>
#endif

#include <TSpline.h>

class PHG4Hit;
class SQHit;
class SQHitVector;
class PHG4HitContainer;

class GeomSvc;

#define NDETPLANES 128

class DPDigiPlane
{
public:
	//!calculates all the derived variables
	void preCalculation();

#ifndef __CINT__
	//!intercepts with a 3-D straight line
	bool intercept(double tx, double ty, double x0, double y0, double z0, G4ThreeVector& pos, double& w);
#endif

	//!@name X, Y, U, V conversion
	//@{
	double getX(double w, double y) const { return w/costh - y*tanth; }
	double getY(double w, double x) const { return w/sinth - x/tanth; }
	double getW(double x, double y) const { return x*costh + y*sinth; }
	//@}

	//!@name get the projection in horizontal/vertical direction
	//@{
	double getW(double x, double y, double z) { return (x-xc)*uVec[0] + (y-yc)*uVec[1] + (z-zc)*uVec[2]; }
	double getH(double x, double y, double z) { return (x-xc)*xVec[0] + (y-yc)*xVec[1] + (z-zc)*xVec[2]; }
	double getV(double x, double y, double z) { return (x-xc)*yVec[0] + (y-yc)*yVec[1] + (z-zc)*yVec[2]; }
	//@}

	//!see if a given position is inside the plane
	bool isInPlane(double x, double y, double z) { return fabs(getH(x, y, z)) < 0.5*planeWidth && fabs(getV(x, y, z)) < 0.5*planeHeight; }

	//stream output
	friend std::ostream& operator << (std::ostream& os, const DPDigiPlane& plane);

public:
	int detectorID;
	std::string detectorGroupName;     //large detector group that this plane belongs to
	std::string detectorName;          //its own specific name

	//!@name geometric specifications
	//@{
	double spacing;
	double cellWidth;
	double angleFromVert;
	double xPrimeOffset;
	double planeWidth;
	double planeHeight;
	double overlap;
	int nElements;
	int triggerLv;
	//@}

	//!@name 3-D position and rotations
	//@{
	double xc, yc, zc;
	double rX, rY, rZ;
	///@}

	//!@name pre-calculated variabels for projections
	//@{
	double wc;         //!< center of the plane in its measuring direction
	double costh, sinth, tanth; //!< pre-calculated  sin, cos, tan
	double nVec[3];    //!< perpendicular to plane
	double uVec[3];    //!< measuring direction
	double vVec[3];    //!< perpendicular to measuring direction
	double xVec[3];    //!< X-axis, horizontal
	double yVec[3];    //!< Y-axis, vertical
	double rotM[3][3]; //!< rotation matrix
	//@}

	//! @name digitization/realization into hits
	//@{
	//TODO: will implement resolution/efficiency/RT
	std::vector<double> efficiency;
	std::vector<double> resolution;
	TSpline3 RT;
	//@}
};

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
	void digitize(std::string detectorGroupName, PHG4Hit& g4hit);

	//!realization process
	bool realize(SQHit& dHit);

	//!get the detectorID by detectorName
	int getDetectorID(std::string detectorName) { return map_detectorID[detectorName]; }

	//!get the detectorName by detectorID
	std::string getDetectorName(int detectorID) { return digiPlanes[detectorID].detectorName; }

	//!Get the trigger level by detectorID
	int getTriggerLv(int detectorID) { return digiPlanes[detectorID].triggerLv; }

	//!Get the digi plane object by ID
	DPDigiPlane& getDigiPlane(int detectorID) { return digiPlanes[detectorID]; }

	//!
	std::string toGroupName(std::string in);

private:

	//!array of digi planes
	//boost::array<DPDigiPlane, NDETPLANES+1> digiPlanes;

	std::vector<DPDigiPlane> digiPlanes;

	//!map from geant sensitive detector name to the list of detectorIDs
	std::map<std::string, std::vector<int> > map_groupID;

	//!map from detectorName to detectorID
	std::map<std::string, int> map_detectorID;

	//!
	std::map<std::string, std::string> map_g4name_group;

	//!
	std::map<std::string, std::string> map_dname_group;

	//!GeomSvc
	GeomSvc *p_geomSvc;

	//!
	SQHitVector *digits;
};

#endif
