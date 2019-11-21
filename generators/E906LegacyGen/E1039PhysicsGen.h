#ifndef __E1039PhysicsGen_H__
#define __E1039PhysicsGen_H__

#include <phhepmc/PHHepMCGenHelper.h>
#ifndef __CINT__
#include <Pythia8/Pythia.h>
#endif
//#include <LHAPDF/LHAPDF.h>
#ifndef __CINT__
#include <gsl/gsl_rng.h>
#endif

#include <iostream>
#include <string>
#include <TDatabasePDG.h>
#include <TParticlePDG.h>
#include <TGenPhaseSpace.h>
//#include <Geant4/G4ParticleTable.hh>

#include <g4main/PHG4ParticleGeneratorBase.h>

class PHCompositeNode;
class E906VertexGen;
class BeamlineObject;
class PHG4InEvent;
class PHG4Particle;
class MCDimuon;
class PHG4ParticleGeneratorBase;
class BeamlineObject;
class SQDimuonTruthInfoContainer;

//==========
class E1039PhysicsGen: public PHG4ParticleGeneratorBase
{
public:
    E1039PhysicsGen();
    virtual ~E1039PhysicsGen();

    
    int Init(PHCompositeNode* topNode);
    int InitRun(PHCompositeNode* topNode);
    int process_event(PHCompositeNode* topNode);
    //void GeneratePrimaries(G4Event* anEvent);

    //!Various generators
    //@{
    int generateDrellYan(PHCompositeNode *topNode, TVector3 vtx, const double pARatio, double luminosity);
    void generateJPsi();
    void generatePsip();
    void generateDarkPhotonFromEta();
    int generatePythia(PHCompositeNode *topNode, TVector3 vtx, const double pARatio);
    int generateCustomDimuon(PHCompositeNode *topNode, TVector3 vtx, const double pARatio);
    void generatePythiaSingle();
    void generateGeant4Single();
    void generateTestSingle();
    void generatePhaseSpace();
    void generateExternal();
    void generateDebug();
    //@}

    //!Dimuon phase space generator
     bool generateDimuon(double mass, double xF, MCDimuon& dimuon, bool angular = false);

     //! Run-accumulated variables
    //@{
    Int_t nEventsThrown;
    Int_t nEventsPhysics;
    Int_t nEventsAccepted;
    //@}
    
    //swith for the generators; Abi
    //@
    void enablePythiaDimuon(){_PythiaDimuon = true;}
    bool _PythiaDimuon;
    void enableCustomDimuon(){_CustomDimuon = true;}
    bool _CustomDimuon;
    void enableDrellYanGen(){_DrellYanGen = true;}
    bool _DrellYanGen;
    bool drellyanMode;
    //@

 private:

    E906VertexGen* _vertexGen;
    PHG4InEvent *ineve ;
    SQDimuonTruthInfoContainer * dimuon_info;
    //MCDimuon* dimuon_info;

    //Pythia generator
    Pythia8::Pythia ppGen;    //!< Pythia pp generator
    Pythia8::Pythia pnGen;    //!< Pythia pn generator

    //!pointer to the current G4Event
    // G4Event* theEvent;
   
    //!particle gun
    //G4ParticleGun* particleGun;
    
    //!Pointer to the particle table
    //G4ParticleTable* particleDict;
   
    //!ROOT phase space generator
    TGenPhaseSpace phaseGen;

    /* //!ROOT TH2D based 2-D interpolation */
    // TH2D* lut;

    /* //!PDFs */
    /* LHAPDF::PDF* pdf; */
    
  
    /*
    //!pointer to the real generator
    typedef void (DPPrimaryGeneratorAction::*GenPtr)();
    GenPtr p_generator;
   
    //!Used for external input
    //@{
    TFile* externalInputFile;
    TTree* externalInputTree;
    int externalEventID;
    int lastFlushPosition;
    int nExternalParticles;
    int externalParticlePDGs[10000];
    TClonesArray* externalPositions;
    TClonesArray* externalMomentums;
    //@}
    */
    //!Common particles to save time
    //@{
    /* G4ParticleDefinition* proton; */
    /* G4ParticleDefinition* mup; */
    /* G4ParticleDefinition* mum; */
    /* G4ParticleDefinition* ep; */
    /* G4ParticleDefinition* em; */
    /* G4ParticleDefinition* pip; */
    /* G4ParticleDefinition* pim; */
    /* G4ParticleDefinition* testPar[2]; */
    /* G4ParticleDefinition* finalPar[2]; */

    /* TParticlePDG* mup; */
    /* TParticlePDG* mum; */
    //@}
    double massMin = 0.22;
    double massMax = 10.;
    double x1Min = 0.;
    double x1Max = 1.;
    double x2Min = 0.;
    double x2Max = 1.;
    double xfMin = -1.;
    double xfMax = 1.;
    double  cosThetaMin = -1.;
    double  cosThetaMax = 1. ;
    double zOffsetMin = -1.;
    double zOffsetMax = 1.;
};

//========




#endif

