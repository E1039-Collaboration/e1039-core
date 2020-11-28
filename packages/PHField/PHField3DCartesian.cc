#include "PHField3DCartesian.h"

#include <TFile.h>
#include <TNtuple.h>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <iostream>
#include <cassert>
#include <fstream>

using namespace std;
using namespace CLHEP;  // units

#define UNIT_LENGTH cm
#define UNIT_FIELD tesla

//#define _DEBUG_ON

namespace {
	typedef boost::tuple<double, double, double> trio;
}

PHField3DCartesian::PHField3DCartesian(const string &fname, const float magfield_rescale)
  : filename(fname)
	, xvals()
	, yvals()
	, zvals()
  , xmin(1000000)
  , xmax(-1000000)
  , ymin(1000000)
  , ymax(-1000000)
  , zmin(1000000)
  , zmax(-1000000)
  , xstepsize(NAN)
  , ystepsize(NAN)
  , zstepsize(NAN)
  , xkey_save(NAN)
  , ykey_save(NAN)
  , zkey_save(NAN)
  , cache_hits(0)
  , cache_misses(0)
{
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      for (int k = 0; k < 2; k++)
      {
        for (int l = 0; l < 3; l++)
        {
          xyz[i][j][k][l] = NAN;
          bf[i][j][k][l] = NAN;
        }
      }
    }
  }
  cout << "\n================ Begin Construct Mag Field =====================" << endl;
  cout << "\n-----------------------------------------------------------"
         << "\n      Magnetic field Module - Verbosity:"
         << "\n-----------------------------------------------------------\n";

  bool root_input = false;
  if(root_input) {
		// open file
		TFile *rootinput = TFile::Open(filename.c_str());
		if (!rootinput)
		{
			cout << "\n could not open " << filename << " exiting now" << endl;
			exit(1);
		}
		cout << "\n ---> "
							"Reading the field grid from "
					 << filename << " ... " << endl;
		rootinput->cd();

		//  get root NTuple objects
		TNtuple *field_map = (TNtuple *) gDirectory->Get("ntuple");
		Float_t ROOT_X, ROOT_Y, ROOT_Z;
		Float_t ROOT_BX, ROOT_BY, ROOT_BZ;
		field_map->SetBranchAddress("x", &ROOT_X);
		field_map->SetBranchAddress("y", &ROOT_Y);
		field_map->SetBranchAddress("z", &ROOT_Z);
		field_map->SetBranchAddress("Bx", &ROOT_BX);
		field_map->SetBranchAddress("By", &ROOT_BY);
		field_map->SetBranchAddress("Bz", &ROOT_BZ);

		for (int i = 0; i < field_map->GetEntries(); i++)
		{
			field_map->GetEntry(i);
			trio coord_key(ROOT_X * UNIT_LENGTH, ROOT_Y * UNIT_LENGTH, ROOT_Z * UNIT_LENGTH);
			trio field_val(ROOT_BX * UNIT_FIELD, ROOT_BY * UNIT_FIELD, ROOT_BZ * UNIT_FIELD);
			xvals.insert(ROOT_X * UNIT_LENGTH);
			yvals.insert(ROOT_Y * UNIT_LENGTH);
			zvals.insert(ROOT_Z * UNIT_LENGTH);
			fieldmap[coord_key] = field_val;

			if (rootinput) rootinput->Close();
		}
  } else {
    ifstream file( filename.c_str() );
    if (!file.good())
      cout << "Field map input file error." << endl;

    char buffer[256];
    file.getline(buffer,256);

    float ROOT_X, ROOT_Y, ROOT_Z;
    float ROOT_BX, ROOT_BY, ROOT_BZ;
    while (file >> ROOT_X >> ROOT_Y >> ROOT_Z >> ROOT_BX >> ROOT_BY >> ROOT_BZ) {
      trio coord_key(ROOT_X * UNIT_LENGTH, ROOT_Y * UNIT_LENGTH, ROOT_Z * UNIT_LENGTH);
      trio field_val(ROOT_BX * magfield_rescale * UNIT_FIELD, ROOT_BY * magfield_rescale * UNIT_FIELD, ROOT_BZ * magfield_rescale * UNIT_FIELD);
    	//cout << ROOT_X << " " << ROOT_Y << " " << ROOT_Z << endl;
    	//cout << ROOT_BX << " " << ROOT_BY << " " << ROOT_BZ << endl;
      xvals.insert(ROOT_X * UNIT_LENGTH);
      yvals.insert(ROOT_Y * UNIT_LENGTH);
      zvals.insert(ROOT_Z * UNIT_LENGTH);
      fieldmap[coord_key] = field_val;
    }
    file.close();
  }

  xmin = *(xvals.begin());
  xmax = *(xvals.rbegin());

  ymin = *(yvals.begin());
  ymax = *(yvals.rbegin());
  //TODO comment out for now, figure out why
//  if (ymin != xmin || ymax != xmax)
//  {
//    cout << "PHField3DCartesian: Compiler bug!!!!!!!! Do not use inlining!!!!!!" << endl;
//    cout << "exiting now - recompile with -fno-inline" << endl;
//    exit(1);
//  }

//  yuhw @ 10/10/18
//  if (magfield_rescale != 1.0)
//  {
//    cout << "PHField3DCartesian: Rescale not implemented" << endl;
//    exit(1);
//  }

  zmin = *(zvals.begin());
  zmax = *(zvals.rbegin());

  xstepsize = (xmax - xmin) / (xvals.size() - 1);
  ystepsize = (ymax - ymin) / (yvals.size() - 1);
  zstepsize = (zmax - zmin) / (zvals.size() - 1);

  std::cout << "  its demensions and ranges of the field: " << fieldmap.size() << std::endl;
  std::cout << "    X: " << xvals.size() << " bins, " << xmin/cm << " cm -- " << xmax/cm << " cm." << std::endl;
  std::cout << "    Y: " << xvals.size() << " bins, " << ymin/cm << " cm -- " << ymax/cm << " cm." << std::endl;
  std::cout << "    Z: " << xvals.size() << " bins, " << zmin/cm << " cm -- " << zmax/cm << " cm." << std::endl;

#ifdef _DEBUG_ON
  auto key = boost::make_tuple(0, 0, 0);
  auto magval = fieldmap.find(key);
  if(magval!=fieldmap.end()) {
  	cout
		<< "DEBUG: "
		<< (magval->second).get<0>() << ", "
		<< (magval->second).get<1>() << ", "
		<< (magval->second).get<2>() << ", "
		<< endl;
  }
#endif


  cout << "\n================= End Construct Mag Field ======================\n"
       << endl;
}

PHField3DCartesian::~PHField3DCartesian()
{
  //   cout << "PHField3DCartesian: cache hits: " << cache_hits
  //        << " cache misses: " << cache_misses
  //        << endl;
}

void PHField3DCartesian::GetFieldValue(const double point[4], double *Bfield) const
{
	//TODO Test!
//  cout << "untested code - I don't know if this is being used, drop me a line (with the field) and I test this"
//       << endl
//       << "   Chris P." << endl;
//  assert(0);

  static double xsav = -1000000.;
  static double ysav = -1000000.;
  static double zsav = -1000000.;

  double x = point[0];
  double y = point[1];
  double z = point[2];

  Bfield[0] = 0.0;
  Bfield[1] = 0.0;
  Bfield[2] = 0.0;
  if (!isfinite(x) || !isfinite(y) || !isfinite(z))
  {
    static int ifirst = 0;
    if (ifirst < 10)
    {
      cout << "PHField3DCartesian::GetFieldValue: "
           << "Invalid coordinates: "
           << "x: " << x
           << ", y: " << y
           << ", z: " << z
           << " bailing out returning zero bfield"
           << endl;
      cout << "previous point: "
           << "x: " << xsav
           << ", y: " << ysav
           << ", z: " << zsav
           << endl;
      ifirst++;
    }
    return;
  }
  xsav = x;
  ysav = y;
  zsav = z;

  if (point[0] < xmin || point[0] > xmax ||
      point[1] < ymin || point[1] > ymax ||
      point[2] < zmin || point[2] > zmax)
  {
    //std::cout << " sdsadsadsadasdasdasdas" << std::endl;
    return;
  }
  set<double>::const_iterator it = xvals.lower_bound(x);
  if (it == xvals.begin())
  {
    cout << "x too small - outside range: " << x << endl;
    return;
  }
  double xkey[2];
  xkey[0] = *it;
  --it;
  xkey[1] = *it;

  it = yvals.lower_bound(y);
  if (it == yvals.begin())
  {
    cout << "y too small - outside range: " << y << endl;
    return;
  }
  double ykey[2];
  ykey[0] = *it;
  --it;
  ykey[1] = *it;

  it = zvals.lower_bound(z);
  if (it == zvals.begin())
  {
    cout << "z too small - outside range: " << z << endl;
    return;
  }
  double zkey[2];
  zkey[0] = *it;
  --it;
  zkey[1] = *it;

  if (xkey_save != xkey[0] ||
      ykey_save != ykey[0] ||
      zkey_save != zkey[0])
  {
    cache_misses++;
    xkey_save = xkey[0];
    ykey_save = ykey[0];
    zkey_save = zkey[0];

    map<boost::tuple<double, double, double>, boost::tuple<double, double, double> >::const_iterator magval;
    trio key;
    for (int i = 0; i < 2; i++)
    {
      for (int j = 0; j < 2; j++)
      {
        for (int k = 0; k < 2; k++)
        {
          key = boost::make_tuple(xkey[i], ykey[j], zkey[k]);
          magval = fieldmap.find(key);
          if (magval == fieldmap.end())
          {
            cout << "could not locate key, x: " << xkey[i] / UNIT_LENGTH
                 << ", y: " << ykey[j] / UNIT_LENGTH
                 << ", z: " << zkey[k] / UNIT_LENGTH << endl;
            return;
          }
          xyz[i][j][k][0] = (magval->first).get<0>();
          xyz[i][j][k][1] = (magval->first).get<1>();
          xyz[i][j][k][2] = (magval->first).get<2>();
          bf[i][j][k][0] = (magval->second).get<0>();
          bf[i][j][k][1] = (magval->second).get<1>();
          bf[i][j][k][2] = (magval->second).get<2>();
#ifdef _DEBUG_ON
          cout << "read x/y/z: "
          		 << xyz[i][j][k][0]/cm << "/"
               << xyz[i][j][k][1]/cm << "/"
               << xyz[i][j][k][2]/cm << " cm"
							 << " bx/by/bz: "
               << bf[i][j][k][0]/tesla << "/"
               << bf[i][j][k][1]/tesla << "/"
               << bf[i][j][k][2]/tesla << " tesla"
							 << endl;
#endif
        }
      }
    }
  }
  else
  {
    cache_hits++;
  }
  // linear extrapolation in cube:
  //Vxyz = 	V000 (1 - x) (1 - y) (1 - z) +
  //V100 x (1 - y) (1 - z) +
  //V010 (1 - x) y (1 - z) +
  //V001 (1 - x) (1 - y) z +
  //V101 x (1 - y) z +
  //V011 (1 - x) y z +
  //V110 x y (1 - z) +
  //V111 x y z

  double xinblock = point[0] - xkey[0];
  double yinblock = point[1] - ykey[0];
  double zinblock = point[2] - zkey[0];
  //   cout << "x/y/z stepsize: " << xstepsize << "/" << ystepsize << "/" << zstepsize << endl;
  //   cout << "x/y/z inblock: " << xinblock << "/" << yinblock << "/" << zinblock << endl;
  for (int i = 0; i < 3; i++)
  {
    Bfield[i] = bf[0][0][0][i] * (xstepsize - xinblock) * (ystepsize - yinblock) * (zstepsize - zinblock) +
                bf[1][0][0][i] * xinblock * (ystepsize - yinblock) * (zstepsize - zinblock) +
                bf[0][1][0][i] * (xstepsize - xinblock) * yinblock * (zstepsize - zinblock) +
                bf[0][0][1][i] * (xstepsize - xinblock) * (ystepsize - yinblock) * zinblock +
                bf[1][0][1][i] * xinblock * (ystepsize - yinblock) * zinblock +
                bf[0][1][1][i] * (xstepsize - xinblock) * yinblock * zinblock +
                bf[1][1][0][i] * xinblock * yinblock * (zstepsize - zinblock) +
                bf[1][1][1][i] * xinblock * yinblock * zinblock;
    Bfield[i] /= (xstepsize*ystepsize*zstepsize);
  }

  return;
}
