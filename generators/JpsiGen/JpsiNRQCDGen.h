#ifndef __JPSI_NRQCD_GEN_H__
#define __JPSI_NRQCD_GEN_H__
#include <string>
#include <TGraph.h>
#include <TGenPhaseSpace.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
class PHCompositeNode;
class PHG4ParticleGeneratorBase;
class PHG4InEvent;
class PHG4Particle;
class PHGenIntegral;
class SQEvent;
class SQMCEvent;
class SQDimuon;
class SQDimuonVector;
class SQPrimaryVertexGen;

/// A J/psi generator based on the cross-section grid data under `e1039-resource/generator/Jpsi_NRQCD`.
/**
 * Events are generated uniformly over xF.
 * The cross section is assigned to the event weight.
 * The absolute scale of the event weight has _not_ been adjusted, and thus
 * you cannot yet estimate the absolute yield using this generator.
 *
 * You can select the following options:
 *  - Three target types:  PROTON, DEUTERON, NITROGEN
 *  - Two PDFs:  CTEQ, NNPDF
 *  - Eight LDMEs:  SMRS_REF, SMRS_FIT, GRV_REF, etc.
 */
class JpsiNRQCDGen: public PHG4ParticleGeneratorBase
{
public:
  typedef enum { PROTON, DEUTERON, NITROGEN } Target_t;
  typedef enum { CTEQ, NNPDF } PDF_t;
  typedef enum { SMRS_REF, SMRS_FIT, GRV_REF, GRV_FIT, JAM_REF, JAM_FIT, xFitter_REF, xFitter_FIT } LDME_t;
  
  JpsiNRQCDGen(const std::string& name = "JpsiNRQCDGen");
  virtual ~JpsiNRQCDGen();
    
  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);

  void SetDataDir(const std::string dir) { _dir_data = dir; }
  void SetTarget(const Target_t tgt) { _tgt = tgt; }
  void SetPDF(const PDF_t pdf) { _pdf = pdf; }
  void SetLDME(const LDME_t ldme) { _ldme = ldme; }
    
  void set_pT0    (const double val) { _pT0   = val; }
  void set_pTpow  (const double val) { _pTpow = val; }
  void set_xfRange(const double xmin, const double xmax) { xfMin = xmin; xfMax = xmax; }

protected:
  int generateJPsi(const TVector3& vtx, const double pARatio, double luminosity);
  bool generateDimuon(double mass, double xF);
  double PickPtE605(const double pTmax);
  double PickPtE906(const double pTmax);
  void InsertMuonPair(const TVector3& vtx);
  void InsertEventInfo(const double xsec, const double weight, const TVector3& vtx);
  
 private:
  std::string _dir_data;
  Target_t _tgt;
  PDF_t _pdf;
  LDME_t _ldme;

  TGraph _gr_xsec_total;
  TGraph _gr_xsec_qqbar;
  TGraph _gr_xsec_gg;
  TGraph _gr_xsec_qg;
  
  SQPrimaryVertexGen* _vertexGen;
  PHG4InEvent* _ineve;
  SQEvent* _evt; //< An output node
  SQMCEvent* _mcevt; //< An output node
  SQDimuonVector* _vec_dim; //< An output node
  PHGenIntegral *_integral_node; //< An output node
  
  SQDimuon* _dim_gen; //< To hold the kinematics of a dimuon generated
  
  TGenPhaseSpace phaseGen;
  
  // Event-generation parameters
  double _pT0;
  double _pTpow;
  //double x1Min = 0.;
  //double x1Max = 1.;
  //double x2Min = 0.;
  //double x2Max = 1.;
  double xfMin = -0.95;
  double xfMax =  0.95;
  //double cosThetaMin = -1.;
  //double cosThetaMax =  1.;
  //double zOffsetMin  = -1.;
  //double zOffsetMax  =  1.;
  
  double _n_gen_acc_evt; //< N of generator-accepted events
  double _n_proc_evt; //< N of processed events
  double _weight_sum; //< Sum of weights
  double _inte_lumi; //< Integrated luminosity
};

#endif // __JPSI_NRQCD_GEN_H__
