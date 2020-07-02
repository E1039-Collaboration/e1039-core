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
  std::cout << "\n================ Begin Construct Mag Field =====================" << std::endl;
  std::cout << "\n-----------------------------------------------------------"
            << "\n      Magnetic field Module - Verbosity:"
            << "\n-----------------------------------------------------------\n";
  for(int i = 0; i < 3; ++i) bgrid[i] = nullptr;

  //load from input file first
  std::vector<FieldRecord> records;
  if(filename.find(".root") != std::string::npos) 
  {
    TFile* inputFile = TFile::Open(filename.c_str());
    if(!inputFile)
    {
      std::cout << "SQField3DCartesian: cannot find the input ROOT field map, abort " << filename << std::endl;
      exit(EXIT_FAILURE);
    }
    std::cout << "SQField3DCartesian: loading field map from ROOT file " << filename << std::endl;

    float x, y, z, Bx, By, Bz;
    TTree* inputTree = (TTree*)inputFile->Get("fieldmap");
    inputTree->SetBranchAddress("x", &x);
    inputTree->SetBranchAddress("y", &y);
    inputTree->SetBranchAddress("z", &z);
    inputTree->SetBranchAddress("Bx", &Bx);
    inputTree->SetBranchAddress("By", &By);
    inputTree->SetBranchAddress("Bz", &Bz);

    unsigned int nRecords = inputTree->GetEntries();
    records.reserve(nRecords);
    for(unsigned int i = 0; i < nRecords; ++i)
    {
      inputTree->GetEntry(i);

      FieldRecord r;
      r.x = x*cm; r.y = y*cm; r.z = z*cm;
      r.Bx = fieldstr*Bx*tesla; r.By = fieldstr*By*tesla; r.Bz = fieldstr*Bz*tesla;
      records.push_back(r);

      xvals.insert(r.x);
      yvals.insert(r.y);
      zvals.insert(r.z);
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
    std::cout << "SQField3DCartesian: loading field map from ASCII file " << filename << std::endl;

    std::string line;
    getline(fin, line);  //header line
    while(getline(fin, line))
    {
      float x, y, z, Bx, By, Bz;
      std::stringstream ss(line);
      ss >> x >> y >> z >> Bx >> By >> Bz;

      FieldRecord r;
      r.x = x*cm; r.y = y*cm; r.z = z*cm;
      r.Bx = fieldstr*Bx*tesla; r.By = fieldstr*By*tesla; r.Bz = fieldstr*Bz*tesla;
      records.push_back(r);

      xvals.insert(r.x);
      yvals.insert(r.y);
      zvals.insert(r.z);
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

  xstepsize = (xmax - xmin)/(xvals.size() - 1);
  ystepsize = (ymax - ymin)/(yvals.size() - 1);
  zstepsize = (zmax - zmin)/(zvals.size() - 1);

  std::cout << "  its demensions and ranges of the field: " << records.size() << std::endl;
  std::cout << "    X: " << xvals.size() << " bins, " << xmin/cm << " cm -- " << xmax/cm << " cm." << std::endl;
  std::cout << "    Y: " << yvals.size() << " bins, " << ymin/cm << " cm -- " << ymax/cm << " cm." << std::endl;
  std::cout << "    Z: " << zvals.size() << " bins, " << zmin/cm << " cm -- " << zmax/cm << " cm." << std::endl;

  //Fill the 3D fieldmap for each dimension
  for(int i = 0; i < 3; ++i)
  {
    bgrid[i] = new TH3D(Form("field_map_%d_%s", i, filename.c_str()), Form("field_map_%d_%s", i, filename.c_str()),
                        xvals.size(), xmin - 0.5*xstepsize, xmax + 0.5*xstepsize,
                        yvals.size(), ymin - 0.5*ystepsize, ymax + 0.5*ystepsize,
                        zvals.size(), zmin - 0.5*zstepsize, zmax + 0.5*zstepsize);
  }

  for(auto iter = records.begin(); iter != records.end(); ++iter)
  {
    bgrid[0]->Fill(iter->x, iter->y, iter->z, iter->Bx);
    bgrid[1]->Fill(iter->x, iter->y, iter->z, iter->By);
    bgrid[2]->Fill(iter->x, iter->y, iter->z, iter->Bz);
  }

  std::cout << "\n================= End Construct Mag Field ======================\n" << std::endl;
}

SQField3DCartesian::~SQField3DCartesian()
{
  for(int i = 0; i < 3; ++i) delete bgrid[i];
}

void SQField3DCartesian::GetFieldValue(const double point[4], double* Bfield) const
{
  for(int i = 0; i < 3; ++i) Bfield[i] = 0.;

  double x = point[0];
  double y = point[1];
  double z = point[2];
  if(!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z) ||
     x < xmin || x > xmax || y < ymin || y > ymax || z < zmin || z > zmax)
  {
    static int ifirst = 0;
    if(ifirst < 10)
    {
      ++ifirst;
      std::cout << "SQField3DCartesian::GetFieldValue: WARNING " << filename << " "
                << "Invalid coordinates: "
                << "x: " << x/cm
                << ", y: " << y/cm
                << ", z: " << z/cm
                << " bailing out returning zero bfield" << std::endl;
    }
    return;
  }

  for(int i = 0; i < 3; ++i) Bfield[i] = bgrid[i]->Interpolate(x, y, z);

  return;
}
