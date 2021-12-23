#ifndef __SQPileupGen_H__
#define __SQPileupGen_H__

#include <vector>
#include <TString.h>
#include <g4main/PHG4ParticleGeneratorBase.h>

class PHCompositeNode;
class PHG4ParticleGeneratorBase;
class PHG4InEvent;
class PHG4Particle;

class TFile;
class TTree;
class TClonesArray;
class TF1;
class TH1;

class SQMCEvent;
class SQPrimaryVertexGen;
class SQEvent;

class ExtParticle
{
 public:
  ExtParticle(int evtID, int pdg, const TVector3& pos, const TVector3& mom);

 public:
  int _evtID;
  int _pdg;
  TVector3 _pos;
  TVector3 _mom;
};

/// An SQ class to pileup the tracks from external file.
class SQPileupGen: public PHG4ParticleGeneratorBase
{
 public:
  SQPileupGen(const std::string& name = "PileupGen");
  virtual ~SQPileupGen();

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);

  void setExtInputFile(const TString& name) { _extFileName = name; } ///< Set external file with track info for pileup
  void setBucketSize(int n) { _bucketSize = n; } ///< Set bucket size for pileup.
  void set_inhibit_threshold (int threshold) {_inhibit_threshold = threshold;} ///< Set inhibit threshold in QIE if using intensity profile for pileup.
  void set_proton_coeff (double coeff) {_proton_coeff = coeff;} ///< Set proton conversion coefficent from QIE count if using intensity profile for pileup.

  TF1* get_beam_intensity_profile() const ///< Return beam intensity profile function for pileup
  {
    return _beam_intensity_profile;
  }

  TH1* get_beam_intensity_profile_histo() const ///< Return beam intensity profile histogram for pileup
  {
    return _beam_intensity_profile_histo;
  }

  void set_beam_intensity_profile(TF1* beamIntensityProfile) 
  {
    _beam_intensity_profile = beamIntensityProfile;
  }

  void set_beam_intensity_profile_histo(TH1* beamIntensityProfile_histo)
  {
    _beam_intensity_profile_histo = beamIntensityProfile_histo;
  }


 private:
  //! read n events and fill the particle container, return false if there is no enough events left
  bool readExtTree(int nEvents);

  //! pointer to the instance of vertex generator
  SQPrimaryVertexGen* _vertexGen;

  //! MC truth container
  PHG4InEvent* _ineve;
  SQMCEvent*   _mcevt;
  SQEvent* _evt; //< An output node


  TF1* _beam_intensity_profile;
  TH1* _beam_intensity_profile_histo;
  //! internal container for quick particle cache
  std::vector<ExtParticle> _extParticles;

  //! Number of protons in each RF bucket
  int _bucketSize;
  int _n_proc_evt; //< N of processed events

  //! External input file and parameters
  TString _extFileName;

  TFile* _extFile;
  TTree* _extTree;

  int _extEvtID;
  int _readIdx;
  int _nExtPar;
  int _inhibit_threshold;
  double _proton_coeff;
  int _extPDG[10000];
  TClonesArray* _extPos;
  TClonesArray* _extMom;

};

#endif
