#include <fstream>
#include <string>
#include <TRandom3.h>
#include <iostream>
#include <cassert>
#include <cstdlib>

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TVector3.h>
#include <TF1.h>
#include <TH1.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>

#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <Geant4/G4ParticleTable.hh>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQEvent_v1.h>

#include "SQPileupGen.h"
#include <E906LegacyVtxGen/SQPrimaryVertexGen.h>

ExtParticle::ExtParticle(int evtID, int pdg, const TVector3& pos, const TVector3& mom):
_evtID(evtID),
_pdg(pdg),
  _pos(pos),
  _mom(mom)
{
}

SQPileupGen::SQPileupGen(const std::string& name):
  PHG4ParticleGeneratorBase(name),
  _ineve(nullptr),
  _mcevt(nullptr),
  _extFile(nullptr),
  _extTree(nullptr),
  _extPos(nullptr),
  _extMom(nullptr),
  _bucketSize(0),
  _evt(0),
  _beam_intensity_profile(nullptr),
  _beam_intensity_profile_histo(nullptr),
  _n_proc_evt(0),
  _inhibit_threshold(5000),
  _proton_coeff(0.03),
  _vertexGen(new SQPrimaryVertexGen)
{
}

SQPileupGen::~SQPileupGen()
{
  if(_vertexGen != nullptr) delete _vertexGen;
}

int SQPileupGen::Init(PHCompositeNode* topNode)
{  
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQPileupGen::InitRun(PHCompositeNode* topNode)
{

  gRandom->SetSeed(PHRandomSeed());
  _ineve = findNode::getClass<PHG4InEvent>(topNode, "PHG4INEVENT");
  if(!_ineve) 
    {
      PHNodeIterator iter(topNode);
      PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
      
      _ineve = new PHG4InEvent();
      dstNode->addNode(new PHIODataNode<PHObject>(_ineve, "PHG4INEVENT", "PHObject"));
    }

  _evt = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! _evt) {
    PHNodeIterator iter(topNode);
    PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

    _evt = new SQEvent_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_evt, "SQEvent", "PHObject"));
  }

  _mcevt = findNode::getClass<SQMCEvent>(topNode, "SQMCEvent");
  if(!_mcevt) 
    {
      PHNodeIterator iter(topNode);
      PHCompositeNode* dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

      _mcevt = new SQMCEvent_v1();
      dstNode->addNode(new PHIODataNode<PHObject>(_mcevt, "SQMCEvent", "PHObject"));
    }

  _extFile = TFile::Open(_extFileName.Data());
  if(!_extFile)
    {
      std::cout << "SQPileupGen: cannot find the input ROOT file, abort " << _extFileName << std::endl;
      exit(EXIT_FAILURE);
    }
  _extTree = (TTree*)_extFile->Get("bkg");

  _extPos = new TClonesArray("TVector3");
  _extMom = new TClonesArray("TVector3");

  _extTree->SetBranchAddress("eventID", &_extEvtID);
  _extTree->SetBranchAddress("n", &_nExtPar);
  _extTree->SetBranchAddress("pdg", _extPDG);
  _extTree->SetBranchAddress("pos", &_extPos);
  _extTree->SetBranchAddress("mom", &_extMom);

  _readIdx = 0;

  _vertexGen->InitRun(topNode);
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQPileupGen::process_event(PHCompositeNode* topNode)
{
  double QIEcount;
  
  if(_beam_intensity_profile) 
    {
      do {
       QIEcount = _beam_intensity_profile->GetRandom();
      } while (QIEcount>_inhibit_threshold);
      _bucketSize  = (int)QIEcount/_proton_coeff;
    }

  if(_beam_intensity_profile_histo)
    {
      do 
	{
       QIEcount = _beam_intensity_profile_histo->GetRandom();
      } while (QIEcount>_inhibit_threshold);
	_bucketSize  = (int)QIEcount/_proton_coeff;
    }

  //if(!readExtTree(_bucketSize)) return Fun4AllReturnCodes::ABORTEVENT;
  if(!readExtTree(_bucketSize)) return Fun4AllReturnCodes::ABORTRUN;

  int eventID_curr = -1;
  int trackID = 0;
  TVector3 priVtx;
  for(auto par = _extParticles.begin(); par != _extParticles.end(); ++par)
    {
      if(par->_evtID != eventID_curr)
	{
	  eventID_curr = par->_evtID;
	}

      int vtxidx = _ineve->AddVtx(par->_pos.X(), par->_pos.Y(), par->_pos.Z(), 0.);

      PHG4Particle* thisPar = new PHG4Particlev2();
      thisPar->set_track_id(trackID++);
      thisPar->set_vtx_id(vtxidx);
      thisPar->set_pid(par->_pdg);
      thisPar->set_px(par->_mom.X());
      thisPar->set_py(par->_mom.Y());
      thisPar->set_pz(par->_mom.Z());
      _ineve->AddParticle(vtxidx, thisPar);
    }

 // std::cout<<"bucket size in pileup generator: "<<_bucketSize<<std::endl;
  _mcevt->set_cross_section(_bucketSize);

  _n_proc_evt++;
  _evt->set_run_id(0);
  _evt->set_spill_id(0);
  _evt->set_event_id(_n_proc_evt);
  _evt->set_qie_rf_intensity(0, QIEcount);
  return Fun4AllReturnCodes::EVENT_OK; 
}

bool SQPileupGen::readExtTree(int nEvents)
{

  if(Verbosity()>0)  std::cout<<"No. of Piledup Events: "<<nEvents<<std::endl;
  _extParticles.clear();
  if(nEvents + _readIdx > _extTree->GetEntries()) return false;

  for(int i = 0; i < nEvents; ++i)
    {
      _extTree->GetEntry(_readIdx++);
      for(int j = 0; j < _nExtPar; ++j)
	{
	  _extParticles.push_back(ExtParticle(_extEvtID, _extPDG[j], *((TVector3*)_extPos->At(j)), *((TVector3*)_extMom->At(j))));
	}
    }

  return true;
}
