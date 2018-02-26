#include "PHFieldSeaQuest.h"

#include <TFile.h>
#include <TNtuple.h>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <iostream>
#include <cassert>
#include <fstream>

using namespace std;
using namespace CLHEP;  // units

typedef boost::tuple<double, double, double> trio;
std::map<boost::tuple<double, double, double>, boost::tuple<double, double, double> > fieldmap;
std::set<double> xvals;
std::set<double> yvals;
std::set<double> zvals;

PHFieldSeaQuest::PHFieldSeaQuest(const std::string &fmag_name, const std::string &kmag_name):
		fmag(fmag_name),
		kmag(kmag_name)

{
  zValues[0] = -204.0*cm;  // front of fmag field map
  zValues[1] = 403.74*cm;  // front of kmag field map
  zValues[2] = 712.0*cm;   // end of fmag field map
  zValues[3] = 1572.26*cm; // end of kmag field map

  kmagZOffset = 1064.26*cm;
}

PHFieldSeaQuest::~PHFieldSeaQuest()
{
  //   cout << "PHFieldSeaQuest: cache hits: " << cache_hits
  //        << " cache misses: " << cache_misses
  //        << endl;
}

void PHFieldSeaQuest::GetFieldValue(const double point[4], double *Bfield) const {

	for(int i=0; i<4; ++i) {
		point[i] /= cm;
	}

	double kmag_point[4] = {point[0], point[1], point[2]-kmagZOffset, point[3]};

  if (point[2]>zValues[0] && point[2]<zValues[1])
  {
    fmag.GetFieldValue( point, Bfield );
  }

  if ((point[2]>zValues[2])&&(point[2]<zValues[3]))
  {
    kmag.GetFieldValue( kmag_point, Bfield );
  }

  if ((point[2]>zValues[1])&&(point[2]<zValues[2]))
  {
  	fmag.GetFieldValue( point, Bfield );
    double xTemp = Bfield[0];
    double yTemp = Bfield[1];
    double zTemp = Bfield[2];
    kmag.GetFieldValue( kmag_point, Bfield );
    Bfield[0] = Bfield[0] + xTemp;
    Bfield[1] = Bfield[1] + yTemp;
    Bfield[2] = Bfield[2] + zTemp;
  }

	for(int i=0; i<3; ++i) {
		Bfield[i] /= tesla;
	}
}
