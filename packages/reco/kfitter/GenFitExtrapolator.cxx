#include "GenFitExtrapolator.h"
#include "GFField.h"

#include <phool/recoConsts.h>
#include <phfield/PHFieldUtility.h>
#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHField.h>

#include <GenFit/FieldManager.h>
#include <GenFit/MaterialEffects.h>
#include <GenFit/TGeoMaterialInterface.h>
#include <GenFit/MeasuredStateOnPlane.h>
#include <GenFit/SharedPlanePtr.h>
#include <GenFit/RKTrackRep.h>

//ROOT
#include <TGeoManager.h>
#include <TMath.h>

//
//#include <G4SystemOfUnits.hh>

#include <memory>
#include <cassert>

//#define _DEBUG_ON

namespace 
{
    //static flag to indicate the initialized has been done
    static bool inited = false;

    //Simple swimming settings 
    static int NSTEPS_TARGET = 100;
    static int NSTEPS_SHIELDING = 50;
    static int NSTEPS_FMAG = 100;

    static double FMAG_LENGTH;
    static double Z_UPSTREAM;

    //initialize global variables
    void initGlobalVariables()
    {
        if(!inited) 
        {
            inited = true;
            recoConsts* rc = recoConsts::instance();
           
            NSTEPS_TARGET = rc->get_IntFlag("NSTEPS_TARGET");
            NSTEPS_SHIELDING = rc->get_IntFlag("NSTEPS_SHIELDING");
            NSTEPS_FMAG = rc->get_IntFlag("NSTEPS_FMAG");

            FMAG_LENGTH = rc->get_DoubleFlag("FMAG_LENGTH");
            Z_UPSTREAM = rc->get_DoubleFlag("Z_UPSTREAM");
        }
    }
}

static const double c_light   = 2.99792458e+8 * m/s;

using namespace std;

GenFitExtrapolator::GenFitExtrapolator():
	pos_i(TVector3()), mom_i(TVector3()), cov_i(TMatrixDSym(5)),
	pos_f(TVector3()), mom_f(TVector3()), cov_f(TMatrixDSym(5)),
	jac_sd2sc(TMatrixD(5,5)), jac_sc2sd(TMatrixD(5,5))
{
    initGlobalVariables();
}

GenFitExtrapolator::~GenFitExtrapolator()
	{}

bool GenFitExtrapolator::init(const PHField* field, const TGeoManager *geom)
{
	iParType = 1;

	assert(field);
	SQGenFit::GFField *fieldMap = new SQGenFit::GFField(field);
	genfit::FieldManager::getInstance()->init(fieldMap);

	_tgeo_manager = const_cast<TGeoManager*>(geom);

#ifdef _DEBUG_ON
	double z_test = 1000;
    LogInfo("");
    {
        double p[4] = {0, 0, z_test*cm, 0};
        double B[3] = {0, 0, 0};
        field->GetFieldValue(p, B);
        cout << "PHField (CLHEP) at Z = " << z_test << endl;
        cout << B[0] << ", " << B[1] << ", " << B[2] << endl;
    }
    {
        genfit::AbsBField *f = genfit::FieldManager::getInstance()->getField();
        TVector3 H = f->get(TVector3(0,0,z_test));
        H *= kilogauss/tesla;
        cout << "genfit::AbsBField (tesla) at Z = " << z_test << endl;
        H.Print();

        z_test = 250;
        H = f->get(TVector3(0,0,z_test));
        H *= kilogauss/tesla;
        cout << "genfit::AbsBField (tesla) at Z = " << z_test << endl;
        H.Print();
    }
#endif

  _tgeo_manager->Export("GenFitExtrapolatorGeom.root");

	genfit::MaterialEffects::getInstance()->init(new genfit::TGeoMaterialInterface());

	return true;
}

void GenFitExtrapolator::setInitialStateWithCov(
		double z_in,
		TMatrixD& state_in, /// 5D state defined on (0, 0, z)
		TMatrixD& cov_in) /// 5D cov
{
#ifdef _DEBUG_ON
	LogInfo("z_in: ") << z_in << endl;
	state_in.Print();
	cov_in.Print();
#endif

	convertSVtoMP(z_in, state_in, mom_i, pos_i);

  if(state_in[0][0] > 0)
  {
      setParticleType(1);
  }
  else
  {
      setParticleType(-1);
  }

  TMatrixD cov_sd(5, 5);
  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          cov_sd[i][j] = cov_in[i][j];

          if(i == 0) cov_sd[i][j] = double(iParType)*cov_sd[i][j];
          if(j == 0) cov_sd[i][j] = double(iParType)*cov_sd[i][j];
      }
  }

  ///convert the error matrix from SD to SC
  TRSDSC(iParType, mom_i, pos_i);
  TMatrixD jac_sd2sc_T = jac_sd2sc;
  jac_sd2sc_T.T();

  TMatrixD cov_sc = jac_sd2sc*cov_sd*jac_sd2sc_T;
  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          cov_i[i][j] = cov_sc[i][j];
      }
  }
}

void GenFitExtrapolator::setParticleType(int type) {
	iParType = type;
}

void GenFitExtrapolator::getFinalStateWithCov(TMatrixD& state_out, TMatrixD& cov_out) {
  convertMPtoSV(mom_f, pos_f, state_out);

  TMatrixD cov_sc(5, 5);
  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          cov_sc[i][j] = cov_f[i][j];

          if(direction == GenFitExtrapolator::Backward)
          {
              if(i == 1) cov_sc[i][j] = -cov_sc[i][j];
              if(j == 1) cov_sc[i][j] = -cov_sc[i][j];
              if(i == 3) cov_sc[i][j] = -cov_sc[i][j];
              if(j == 3) cov_sc[i][j] = -cov_sc[i][j];
          }
      }
  }

  //convert from SC to SD error matrix
  TRSCSD(iParType, mom_f, pos_f);
  TMatrixD jac_sc2sd_T = jac_sc2sd;
  jac_sc2sd_T.T();

  cov_out = jac_sc2sd*cov_sc*jac_sc2sd_T;

  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          if(i == 0) cov_out[i][j] = double(iParType)*cov_out[i][j];
          if(j == 0) cov_out[i][j] = double(iParType)*cov_out[i][j];
      }
  }
}

void GenFitExtrapolator::getPropagator(TMatrixD& prop) {
//  if(fabs(pos_i[2] - pos_f[2]) < 1E-3)
//  {
//      prop.UnitMatrix();
//      return;
//  }
//
//  for(int i = 0; i < 5; i++)
//  {
//      for(int j = 0; j < 5; j++)
//      {
//          prop[i][j] = g4eProp[i][j];
//
//          if(i == 0) prop[i][j] = double(iParType)*prop[i][j];
//          if(j == 0) prop[i][j] = double(iParType)*prop[i][j];
//          if(direction == GenFitExtrapolator::Backward)
//          {
//              if(i == 1) prop[i][j] = -prop[i][j];
//              if(j == 1) prop[i][j] = -prop[i][j];
//              if(i == 3) prop[i][j] = -prop[i][j];
//              if(j == 3) prop[i][j] = -prop[i][j];
//          }
//      }
//  }
//
//  prop = jac_sc2sd*prop*jac_sd2sc;
}

bool GenFitExtrapolator::extrapolateTo(double z_out) {

  ///If the initial or final position is out of the reasonable world
  if(pos_i[2] > 2400 || pos_i[2] < -600 || z_out > 2400 || z_out < -600)
  {
      return false;
  }

  ///if the initial and final z position is the same, don't make the transportation
  if(fabs(pos_i[2] - z_out) < 1E-3)
  {
      mom_f = mom_i;
      pos_f = pos_i;
      cov_f = cov_i;

      return true;
  }

	int pid = iParType > 0 ? -13 : 13;
#ifdef _DEBUG_ON
		cout
		<< "extrapolateToPlane: "
		<< "From: " << pos_i.Z()
		<< " -> " << z_out
		<< endl;
#endif

	auto up_rep = std::unique_ptr<genfit::AbsTrackRep> (new genfit::RKTrackRep(pid));
	auto rep = up_rep.get();

	genfit::SharedPlanePtr destPlane(new genfit::DetPlane(TVector3(0, 0, z_out), TVector3(0, 0, 1)));

	std::unique_ptr<genfit::MeasuredStateOnPlane> currentState = std::unique_ptr < genfit::MeasuredStateOnPlane > (new genfit::MeasuredStateOnPlane(rep));
	//rep->setPosMomCov(*currentState, pos_i, mom_i, cov_i);
	currentState->setPosMom(pos_i, mom_i);
	currentState->setCov(cov_i);

	try {
		travelLength = rep->extrapolateToPlane(*currentState, destPlane);
	} catch (...) {
#ifdef _DEBUG_ON
		cout << "extrapolateToPlane failed!" << endl;
#endif
		return false;
	}

	currentState->getPosMom(pos_f, mom_f);
	cov_f = currentState->getCov();


	return true;
}

//int GenFitExtrapolator::propagate() {
//
//	return -1;
//}

double GenFitExtrapolator::extrapolateToIP() {

  //Store the steps on each point
  TVector3 mom[NSTEPS_FMAG + NSTEPS_TARGET + 1];
  TVector3 pos[NSTEPS_FMAG + NSTEPS_TARGET + 1];

  //Step size in FMAG/target area, unit is cm.
  //FIXME Units
  double step_fmag   = FMAG_LENGTH/NSTEPS_FMAG;//*cm;
  double step_target = fabs(Z_UPSTREAM)/NSTEPS_TARGET;//*cm;

  //Start from FMAG face downstream
  extrapolateTo(FMAG_LENGTH);
  pos[0] = pos_f;
  mom[0] = mom_f;

  //Now make the real swimming
  int iStep = 1;
  for(; iStep <= NSTEPS_FMAG; ++iStep)
  {
      pos_i = pos[iStep-1];
      mom_i = mom[iStep-1];

      extrapolateTo((pos_i[2] - step_fmag));

      pos[iStep] = pos_f;
      mom[iStep] = mom_f;
  }

  for(; iStep < NSTEPS_FMAG+NSTEPS_TARGET+1; ++iStep)
  {
      pos_i = pos[iStep-1];
      mom_i = mom[iStep-1];

      extrapolateTo((pos_i[2] - step_target));

      pos[iStep] = pos_f;
      mom[iStep] = mom_f;
  }

  //Find the one step with minimum DCA
  double dca_min = 1E6;
  for(int i = 0; i < NSTEPS_FMAG+NSTEPS_TARGET+1; ++i)
  {
      double dca = sqrt(pos[i][0]*pos[i][0] + pos[i][1]*pos[i][1]);
      if(dca < dca_min)
      {
          dca_min = dca;
          iStep = i;
      }
  }

  return pos[iStep][2];
}
void GenFitExtrapolator::convertSVtoMP(double z, TMatrixD& state, TVector3& mom, TVector3& pos)
{
	double p = fabs(1. / state[0][0]);
	double pz = p / sqrt(1. + state[1][0] * state[1][0] + state[2][0] * state[2][0]);
	double px = pz * state[1][0];
	double py = pz * state[2][0];

	double x = state[3][0];
	double y = state[4][0];

	pos.SetXYZ(x, y, z);
	mom.SetXYZ(px, py, pz);
}

void GenFitExtrapolator::convertMPtoSV(TVector3& mom, TVector3& pos, TMatrixD& state) {
	TVector3 mom_gev = mom;
	TVector3 pos_cm = pos;

	state[0][0] = double(iParType) / mom_gev.Mag();
	state[1][0] = mom_gev.x() / mom_gev.z();
	state[2][0] = mom_gev.y() / mom_gev.z();

	state[3][0] = pos_cm.x();
	state[4][0] = pos_cm.y();
}

void GenFitExtrapolator::TRSDSC(int charge, TVector3 mom_input, TVector3 pos_input)
{
    ///convert the internal momentum and positon to ROOT vectors and use GeV and cm
    TVector3 mom(mom_input[0], mom_input[1], mom_input[2]);
    TVector3 pos(pos_input[0], pos_input[1], pos_input[2]);

    ///Define the V and W direction of SD coordinate
    TVector3 DJ(1., 0., 0.);
    TVector3 DK(0., 1., 0.);
    TVector3 DI = DJ.Cross(DK);

    ///Calculate the representation of momentum
    double p_inv = 1./mom.Mag();
    double vp = mom.Dot(DJ)/mom.Dot(DI);
    double wp = mom.Dot(DK)/mom.Dot(DI);
    double lambda = TMath::Pi()/2. - mom.Theta();
    //double phi = mom.Phi();

    TVector3 TVW;
    TVW.SetX(1./sqrt(1. + vp*vp + wp*wp));
    TVW.SetY(vp*TVW.X());
    TVW.SetZ(wp*TVW.X());

    TVector3 TN;
    for(int i = 0; i < 3; i++)
    {
        TN[i] = TVW[0]*DI[i] + TVW[1]*DJ[i] + TVW[2]*DK[i];
    }

    double cosl = cos(lambda);
    double cosl1 = 1./cosl;

    TVector3 UN(-TN.Y()*cosl1, TN.X()*cosl1, 0.);
    TVector3 VN(-TN.Z()*UN.Y(), TN.Z()*UN.X(), cosl);

    double UJ = UN.Dot(DJ);
    double UK = UN.Dot(DK);
    double VJ = VN.Dot(DJ);
    double VK = VN.Dot(DK);

    jac_sd2sc.Zero();
    jac_sd2sc[0][0] = 1.;
    jac_sd2sc[1][1] = TVW[0]*VJ;
    jac_sd2sc[1][2] = TVW[0]*VK;
    jac_sd2sc[2][1] = TVW[0]*UJ*cosl1;
    jac_sd2sc[2][2] = TVW[0]*UK*cosl1;
    jac_sd2sc[3][3] = UJ;
    jac_sd2sc[3][4] = UK;
    jac_sd2sc[4][3] = VJ;
    jac_sd2sc[4][4] = VK;

    //Takes cm output kGauss (0.1*tesla)
    genfit::AbsBField *field = genfit::FieldManager::getInstance()->getField();

    if(charge != 0 && field)
    {
        TVector3 H = field->get(pos);
        H *= kilogauss/tesla;
#ifdef _DEBUG_ON
        LogInfo("");
        pos.Print();
        H.Print();
#endif
        double HA = H.Mag();
        double HAM = HA*p_inv*tesla*GeV;
        double HM;
        if(HA < 1E-6)
        {
            HM = 0.;
        }
        else
        {
            HM = charge/HA;
        }

        double Q = -HAM*c_light/(km/ns);
        double sinz = -H.Dot(UN)*HM;
        double cosz = H.Dot(VN)*HM;

        jac_sd2sc[1][3] = -Q*TVW[1]*sinz;
        jac_sd2sc[1][4] = -Q*TVW[2]*sinz;
        jac_sd2sc[2][3] = -Q*TVW[1]*cosz*cosl1;
        jac_sd2sc[2][4] = -Q*TVW[2]*cosz*cosl1;
    }
}

void GenFitExtrapolator::TRSCSD(int charge, TVector3 mom_input, TVector3 pos_input)
{
    ///convert the internal momentum and positon to ROOT vectors and use GeV and cm
    TVector3 mom(mom_input[0], mom_input[1], mom_input[2]);
    TVector3 pos(pos_input[0], pos_input[1], pos_input[2]);

    ///Define the V and W direction of SD coordinate
    TVector3 DJ(1., 0., 0.);
    TVector3 DK(0., 1., 0.);
    TVector3 DI = DJ.Cross(DK);

    ///Calculate the representation of momentum
    double p_inv = 1./mom.Mag();
    //double vp = mom.Dot(DJ)/mom.Dot(DI);
    //double wp = mom.Dot(DK)/mom.Dot(DI);
    double lambda = TMath::Pi()/2. - mom.Theta();
    double phi = mom.Phi();

    double cosl = cos(lambda);
    double sinp = sin(phi);
    double cosp = cos(phi);

    TVector3 TN(cosl*cosp, cosl*sinp, sin(lambda));
    TVector3 TVW;
    TVW.SetX(TN.Dot(DI));
    TVW.SetY(TN.Dot(DJ));
    TVW.SetZ(TN.Dot(DK));

    double T1R = 1./TVW[0];
    double T2R = T1R*T1R;
    TVector3 UN(-sinp, cosp, 0.);
    TVector3 VN(-TN.Z()*UN.Y(), TN.Z()*UN.X(), cosl);

    double UJ = UN.Dot(DJ);
    double UK = UN.Dot(DK);
    double VJ = VN.Dot(DJ);
    double VK = VN.Dot(DK);
    double UI = UN.Dot(DI);
    double VI = VN.Dot(DI);

    jac_sc2sd.Zero();
    jac_sc2sd[0][0] = 1.;
    jac_sc2sd[1][1] = -UK*T2R;
    jac_sc2sd[1][2] = VK*cosl*T2R;
    jac_sc2sd[2][1] = UJ*T2R;
    jac_sc2sd[2][2] = -VJ*cosl*T2R;
    jac_sc2sd[3][3] = VK*T1R;
    jac_sc2sd[3][4] = -UK*T1R;
    jac_sc2sd[4][3] = -VJ*T1R;
    jac_sc2sd[4][4] = UJ*T1R;

    genfit::AbsBField *field = genfit::FieldManager::getInstance()->getField();
    if(charge != 0 && field)
    {
    		TVector3 H = field->get(pos);
    		H *= kilogauss/tesla;

        double HA = H.Mag();
        double HAM = HA*p_inv;
        double HM;
        if(HA < 1E-6)
        {
            HM = 0.;
        }
        else
        {
            HM = charge/HA;
        }

        double Q = -HAM*c_light/(km/ns);
        double sinz = -H.Dot(UN)*HM;
        double cosz = H.Dot(VN)*HM;
        double T3R = Q*T1R*T1R*T1R;

        jac_sc2sd[1][3] = -UI*(VK*cosz - UK*sinz)*T3R;
        jac_sc2sd[1][4] = -VI*(VK*cosz - UK*sinz)*T3R;
        jac_sc2sd[2][3] = UI*(VJ*cosz - UJ*sinz)*T3R;
        jac_sc2sd[2][4] = VI*(VJ*cosz - UJ*sinz)*T3R;
    }
}

void GenFitExtrapolator::print() {
  cout << "Propagating " << iParType << ":" << endl;
  cout << "From "
  		<< "(" << pos_i.X() << ", " << pos_i.Y() << ", "<< pos_i.Z() << ") "
			<< " To "
			<< "(" << pos_f.X() << ", " << pos_f.Y() << ", "<< pos_f.Z() << ") "
			<< endl;

  cout << "Momentum change: From "
  		<< "(" << mom_i.X() << ", " << mom_i.Y() << ", "<< mom_i.Z() << ") "
			<< " To "
			<< "(" << mom_f.X() << ", " << mom_f.Y() << ", "<< mom_f.Z() << ") "
			<< endl;

  cout << "Initial error matrix: " << endl;
  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          cout << cov_i[i][j] << "  ";
      }
      cout << endl;
  }

  cout << "Final error matrix: " << endl;
  for(int i = 0; i < 5; i++)
  {
      for(int j = 0; j < 5; j++)
      {
          cout << cov_f[i][j] << "  ";
      }
      cout << endl;
  }
}
