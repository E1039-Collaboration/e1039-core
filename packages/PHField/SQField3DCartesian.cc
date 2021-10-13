#include "SQField3DCartesian.h"

#include <TFile.h>
#include <TTree.h>

#include <iostream>
#include <fstream>
#include <sstream>

SQField3DCartesian::SQField3DCartesian(const std::string& fname, const float magfield_rescale)
  : filename(fname)
  , fieldstr(magfield_rescale)
  , xvals()
  , yvals()
  , zvals()
  , xsteps(0)
  , ysteps(0)
  , zsteps(0)
  , xmin(1000000)
  , xmax(-1000000)
  , ymin(1000000)
  , ymax(-1000000)
  , zmin(1000000)
  , zmax(-1000000)
  , xstepsize(-1.)
  , ystepsize(-1.)
  , zstepsize(-1.)
{
#ifdef _DEBUG_ON
  std::cout << "\n================ Begin Construct Mag Field =====================" << std::endl;
  std::cout << "\n-----------------------------------------------------------"
            << "\n      Magnetic field Module - Verbosity:"
            << "\n-----------------------------------------------------------\n";
#endif
  //load from input file first
  fpoints.clear();
  if(filename.find(".root") != std::string::npos) 
  {
    TFile* inputFile = TFile::Open(filename.c_str());
    if(!inputFile)
    {
      std::cout << "SQField3DCartesian: cannot find the input ROOT field map, abort " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
#ifdef _DEBUG_ON
    std::cout << "SQField3DCartesian: loading field map from ROOT file " << filename << std::endl;
#endif
    float x, y, z, Bx, By, Bz;
    TTree* inputTree = (TTree*)inputFile->Get("fieldmap");
    inputTree->SetBranchAddress("x", &x);
    inputTree->SetBranchAddress("y", &y);
    inputTree->SetBranchAddress("z", &z);
    inputTree->SetBranchAddress("Bx", &Bx);
    inputTree->SetBranchAddress("By", &By);
    inputTree->SetBranchAddress("Bz", &Bz);

    unsigned int nRecords = inputTree->GetEntries();
    fpoints.reserve(nRecords);
    for(unsigned int i = 0; i < nRecords; ++i)
    {
      inputTree->GetEntry(i);

      FieldPoint p;
      p.x = x*cm; p.y = y*cm; p.z = z*cm;
      p.B.SetXYZ(fieldstr*Bx*tesla, fieldstr*By*tesla, fieldstr*Bz*tesla);
      fpoints.push_back(p);

      xvals.insert(p.x);
      yvals.insert(p.y);
      zvals.insert(p.z);
    }

    inputFile->Close();
  }
  else  //load from ascii input
  {
    std::ifstream fin(filename.c_str());
    if(!fin)
    {
      std::cout << "SQField3DCartesian: cannot find the input ASCII field map, abort " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
#ifdef _DEBUG_ON
    std::cout << "SQField3DCartesian: loading field map from ASCII file " << filename << std::endl;
#endif
    std::string line;
    getline(fin, line);  //header line
    while(getline(fin, line))
    {
      float x, y, z, Bx, By, Bz;
      std::stringstream ss(line);
      ss >> x >> y >> z >> Bx >> By >> Bz;

      FieldPoint p;
      p.x = x*cm; p.y = y*cm; p.z = z*cm;
      p.B.SetXYZ(fieldstr*Bx*tesla, fieldstr*By*tesla, fieldstr*Bz*tesla);
      fpoints.push_back(p);

      xvals.insert(p.x);
      yvals.insert(p.y);
      zvals.insert(p.z);
    }

    fin.close();
  }

  //Extract information on each dimension
  xmin = *(xvals.begin());
  xmax = *(xvals.rbegin());
  ymin = *(yvals.begin());
  ymax = *(yvals.rbegin());
  zmin = *(zvals.begin());
  zmax = *(zvals.rbegin());

  xsteps = xvals.size();
  ysteps = yvals.size();
  zsteps = zvals.size();

  xstepsize = (xmax - xmin)/(xsteps - 1);
  ystepsize = (ymax - ymin)/(ysteps - 1);
  zstepsize = (zmax - zmin)/(zsteps - 1);

  //NOTE here we assume the input files (text or root) are pre-sorted. Otherwise we need to sort the fpoints vector
  //using something like this: 
  //std::sort(fpoints.begin(), fpoints.end(), [&](FieldPoint a, FieldPoint b) 
  //{ return (int((a.z-zmin)/zstepsize) + zsteps*(int((a.y-ymin)/ystepsize)) + ysteps*(int((a.x-xmin)/xstepsize))) < 
  //         (int((b.z-zmin)/zstepsize) + zsteps*(int((b.y-ymin)/ystepsize)) + ysteps*(int((b.x-xmin)/xstepsize))); })

#ifdef _DEBUG_ON
  std::cout << "  its dimensions and ranges of the field: " << fpoints.size() << std::endl;
  std::cout << "    X: " << xsteps << " bins, " << xmin/cm << " cm -- " << xmax/cm << " cm, step size = " << xstepsize/cm << " cm." << std::endl;
  std::cout << "    Y: " << ysteps << " bins, " << ymin/cm << " cm -- " << ymax/cm << " cm, step size = " << ystepsize/cm << " cm." << std::endl;
  std::cout << "    Z: " << zsteps << " bins, " << zmin/cm << " cm -- " << zmax/cm << " cm, step size = " << zstepsize/cm << " cm." << std::endl;

  std::cout << "\n================= End Construct Mag Field ======================\n" << std::endl;
#endif
}

SQField3DCartesian::~SQField3DCartesian()
{}

int SQField3DCartesian::GetGlobalIndex(int xIdx, int yIdx, int zIdx) const
{
  return zIdx + zsteps*(yIdx + ysteps*xIdx);
}

void SQField3DCartesian::GetFieldValue(const double point[4], double* Bfield) const
{
  //This 3D intepolation algorithm is based on the wiki link http://en.wikipedia.org/wiki/Trilinear_interpolation

  for(int i = 0; i < 3; ++i) Bfield[i] = 0.;
  double x = point[0];
  double y = point[1];
  double z = point[2];

  int ubx = int((xsteps-1)*(x - xmin)/(xmax - xmin));
  int uby = int((ysteps-1)*(y - ymin)/(ymax - ymin));
  int ubz = int((zsteps-1)*(z - zmin)/(zmax - zmin));
  int obx = ubx + 1;
  int oby = uby + 1;
  int obz = ubz + 1;

  if(ubx < 0 || uby < 0 || ubz < 0 ||
     obx >= xsteps || oby >= ysteps || obz >= zsteps)
  {
    /*
    static int ifirst = 0;
    if(ifirst < 10)
    {
      ++ifirst;
      std::cout << "SQField3DCartesian::GetFieldValue: WARNING " << filename << " "
                << "Out-of-boundary coordinates: "
                << "x: " << x/cm
                << ", y: " << y/cm
                << ", z: " << z/cm
                << " bailing out returning zero bfield" << std::endl;
    }
    */
    return;
  }

  int idx000 = GetGlobalIndex(ubx, uby, ubz);
  int idx001 = GetGlobalIndex(ubx, uby, obz);
  int idx010 = GetGlobalIndex(ubx, oby, ubz);
  int idx011 = GetGlobalIndex(ubx, oby, obz);
  int idx100 = GetGlobalIndex(obx, uby, ubz);
  int idx101 = GetGlobalIndex(obx, uby, obz);
  int idx110 = GetGlobalIndex(obx, oby, ubz);
  int idx111 = GetGlobalIndex(obx, oby, obz);

  double xp = (x - fpoints[idx000].x)/xstepsize;
  double yp = (y - fpoints[idx000].y)/ystepsize;
  double zp = (z - fpoints[idx000].z)/zstepsize;

  TVector3 i1 = fpoints[idx000].B*(1. - zp) + fpoints[idx001].B*zp;
  TVector3 i2 = fpoints[idx010].B*(1. - zp) + fpoints[idx011].B*zp;
  TVector3 j1 = fpoints[idx100].B*(1. - zp) + fpoints[idx101].B*zp;
  TVector3 j2 = fpoints[idx110].B*(1. - zp) + fpoints[idx111].B*zp;

  TVector3 w1 = i1*(1. - yp) + i2*yp;
  TVector3 w2 = j1*(1. - yp) + j2*yp;

  TVector3 res = w1*(1. - xp) + w2*xp;
  Bfield[0] = res.X();
  Bfield[1] = res.Y();
  Bfield[2] = res.Z();
}
