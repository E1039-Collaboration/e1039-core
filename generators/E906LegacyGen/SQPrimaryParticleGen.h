/*====================================================================
Author: Abinash Pun, Kun Liu
Nov, 2019
Goal: Import the physics generator of E906 experiment (DPSimPrimaryGeneratorAction)
from Kun to E1039 experiment in Fun4All framework
=========================================================================*/
#ifndef __SQPrimaryParticleGen_H__
#define __SQPrimaryParticleGen_H__

#ifndef __CINT__
#include <Pythia8/Pythia.h>
#endif
#include <LHAPDF/LHAPDF.h>
#ifndef __CINT__
#include <gsl/gsl_rng.h>
#endif

#include <TGenPhaseSpace.h>
#include <g4main/PHG4ParticleGeneratorBase.h>

class PHCompositeNode;
class PHG4ParticleGeneratorBase;
class PHG4InEvent;
class PHG4Particle;

class SQMCEvent;
class SQDimuon;
class SQDimuonVector;
class SQPrimaryVertexGen;

//==========
class SQPrimaryParticleGen: public PHG4ParticleGeneratorBase
{
public:
    SQPrimaryParticleGen();
    virtual ~SQPrimaryParticleGen();

    
    int Init(PHCompositeNode* topNode);
    int InitRun(PHCompositeNode* topNode);
    int process_event(PHCompositeNode* topNode);
    //void GeneratePrimaries(G4Event* anEvent);

    //!Various generators
    //@{
    int generateDrellYan(PHCompositeNode *topNode, TVector3 vtx, const double pARatio, double luminosity);
    int generateJPsi(PHCompositeNode *topNode, TVector3 vtx, const double pARatio, double luminosity);
    int generatePsip(PHCompositeNode *topNode, TVector3 vtx, const double pARatio, double luminosity);
    // void generateDarkPhotonFromEta();
    int generatePythia(PHCompositeNode *topNode, TVector3 vtx, const double pARatio);
    int generateCustomDimuon(PHCompositeNode *topNode, TVector3 vtx, const double pARatio);
  
    //@}

    //!Dimuon phase space generator
     bool generateDimuon(double mass, double xF, bool angular = false);

     //! Run-accumulated variables
    //@{
    Int_t nEventsThrown;
    Int_t nEventsPhysics;
    Int_t nEventsAccepted;
    //@}
    
    //swith for the generators; Abi
    //@
    void enablePythia(){_Pythia = true;}
    bool _Pythia;
    void enableCustomDimuon(){_CustomDimuon = true;}
    bool _CustomDimuon;
    void enableDrellYanGen(){_DrellYanGen = true;}
    bool _DrellYanGen;
    bool drellyanMode;
    void enableJPsiGen(){_JPsiGen = true;}
    bool _JPsiGen;
    void enablePsipGen(){_PsipGen = true;}
    bool _PsipGen;

    void set_xfRange(const double xmin, const double xmax){
      xfMin = xmin;
      xfMax = xmax;
    }
    void set_massRange(const double mmin, const double mmax){
      massMin = mmin;
      massMax = mmax;
    }
    //@

 private:

    SQPrimaryVertexGen* _vertexGen;
    PHG4InEvent *ineve;
    SQMCEvent* _mcevt; //< An output node
    SQDimuonVector* _vec_dim; //< An output node

    SQDimuon* _dim_gen; //< To hold the kinematics of a dimuon generated

    //Pythia generator
    Pythia8::Pythia ppGen;    //!< Pythia pp generator
    Pythia8::Pythia pnGen;    //!< Pythia pn generator

  
    //!ROOT phase space generator
    TGenPhaseSpace phaseGen;


    //!PDFs
    LHAPDF::PDF* pdf;
    
  
    //some initializations
  
    double massMin = 0.22;
    double massMax = 10.;
    double x1Min = 0.;
    double x1Max = 1.;
    double x2Min = 0.;
    double x2Max = 1.;
    double xfMin = -1.;
    double xfMax = 1.;
    double cosThetaMin = -1.;
    double cosThetaMax = 1. ;
    double zOffsetMin = -1.;
    double zOffsetMax = 1.;

    void InsertMuonPair(TVector3& vtx);
    void InsertEventInfo(double xsec, TVector3& vtx);
};

//========




#endif

