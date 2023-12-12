#include "PHFieldSeaQuest.h"
#include <phool/recoConsts.h>
#include <TFile.h>
#include <TNtuple.h>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <iostream>
#include <cassert>
#include <fstream>

using namespace std;
using namespace CLHEP;  // units

PHFieldSeaQuest::PHFieldSeaQuest(
		const std::string &fmag_name,
		const std::string &kmag_name,
		const double fmag_scale,
		const double kmag_scale,
		const double targermag_y):
		fmag(fmag_name, fmag_scale),
		kmag(kmag_name, kmag_scale),
		targetmag(targermag_y)

{
  kmagZOffset = recoConsts::instance()->get_DoubleFlag("Z_KMAG_BEND")*cm;

  zValues[0] = fmag.GetZMin();                // front of fmag field map
  zValues[1] = kmag.GetZMin() + kmagZOffset;  // front of kmag field map
  zValues[2] = fmag.GetZMax();                // end of fmag field map
  zValues[3] = kmag.GetZMax() + kmagZOffset;  // end of kmag field map

  targetmag.set_mean_x(0*cm);
  targetmag.set_mean_y(0*cm);
  targetmag.set_mean_z(-300*cm);
}

PHFieldSeaQuest::~PHFieldSeaQuest()
{
  //   cout << "PHFieldSeaQuest: cache hits: " << cache_hits
  //        << " cache misses: " << cache_misses
  //        << endl;
}

void PHFieldSeaQuest::GetFieldValue(const double point[4], double *Bfield) const 
{
  double kmag_point[4] = {point[0], point[1], point[2]-kmagZOffset, point[3]};

  if(point[2] < zValues[0]) 
  {
    targetmag.GetFieldValue(point, Bfield);
  }
  else if(point[2] > zValues[0] && point[2] < zValues[1])
  {
    fmag.GetFieldValue(point, Bfield);
  } 
  else if(point[2] > zValues[2] && point[2] < zValues[3]) 
  {
    kmag.GetFieldValue(kmag_point, Bfield);
  } 
  else if(point[2] > zValues[1] && point[2] < zValues[2]) 
  {
    fmag.GetFieldValue(point, Bfield);
    double xTemp = Bfield[0];
    double yTemp = Bfield[1];
    double zTemp = Bfield[2];

    kmag.GetFieldValue(kmag_point, Bfield);
    Bfield[0] = Bfield[0] + xTemp;
    Bfield[1] = Bfield[1] + yTemp;
    Bfield[2] = Bfield[2] + zTemp;
  } 
  else 
  {
    Bfield[0] = 0.;
    Bfield[1] = 0.;
    Bfield[2] = 0.;
  }

  /*
  std::cout << "GetFieldValue: " 
                << "x: " << point[0]/cm
                << ", y: " << point[1]/cm
                << ", z: " << point[2]/cm
                << ", Bx: " << Bfield[0]/tesla
                << ", By: " << Bfield[1]/tesla
                << ", Bz: " << Bfield[2]/tesla << std::endl;
  */
}

void PHFieldSeaQuest::identify(std::ostream& os) const {
	os << "PHFieldSeaQuest::identify: " << "-------" << endl;
  double point[4] =   {0, 0, 0, 0};
  double bfield[3] =  {0, 0, 0};

  for(point[2] = -500*cm; point[2] < 1500*cm; point[2] += 1*cm) {
  	this->GetFieldValue(point, bfield);
  	os << point[2]/cm << ", " << bfield[1]/tesla << endl;
  }
}
