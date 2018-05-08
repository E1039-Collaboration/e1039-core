
#include "PHFieldRegionalConst.h"

//root framework
#include <TFile.h>
#include <TNtuple.h>

#include <set>
#include <iostream>

using namespace std;
using namespace CLHEP;// units

PHFieldRegionalConst::PHFieldRegionalConst( const string &filename, const int verb, const float magfield_rescale) :
    PHField(verb),
		maxy_(4*cm), miny_(-4*cm),
		maxr_(22.225*cm), minr_(0*cm),
		field_val_(5*tesla)
{
}

void PHFieldRegionalConst::GetFieldValue(const double point[4], double *Bfield ) const
{
  double x = point[0];
  double y = point[1];
  double z = point[2];
	double r = sqrt(x*x + z*z);

	Bfield[0] = 0;
	Bfield[1] = 0;
	Bfield[2] = 0;

	if( r > minr_ and r < maxr_ and
			y > miny_ and y < maxy_) {
		Bfield[1] = field_val_;
	}

//	std::cout
//	<< "PHFieldRegionalConst::GetFieldValue: "
//	<< "{" << x/cm << ", " << y/cm << ", " << z/cm << ","  << r/cm << "}"
//	<< "\t {" << Bfield[0]/tesla << ", " << Bfield[1]/tesla << ", " << Bfield[2]/tesla << "}"
//	<< std::endl;

  return;
}

