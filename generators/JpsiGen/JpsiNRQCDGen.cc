#include <fstream>
#include <string>
#include <TSystem.h>
#include <TRandom3.h>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/recoConsts.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>
#include <phool/PHRandomSeed.h>
#include <phool/getClass.h>
#include <phgeom/PHGeomUtility.h>
#include <boost/format.hpp>
#include <phhepmc/PHGenIntegralv1.h>
#include <g4main/PHG4ParticleGeneratorBase.h>
#include <g4main/PHG4InEvent.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>
#include <g4main/PHG4VtxPoint.h>
#include <g4main/PHG4TruthInfoContainer.h>
//#include <gsl/gsl_randist.h>
#include <Geant4/G4ParticleTable.hh>
#include <Geant4/G4PhysicalConstants.hh>
#include <interface_main/SQEvent_v2.h>
#include <interface_main/SQMCEvent_v1.h>
#include <interface_main/SQDimuon_v1.h>
#include <interface_main/SQDimuonVector_v1.h>
#include <UtilAna/UtilDimuon.h>
#include <E906LegacyVtxGen/SQPrimaryVertexGen.h>
#include "JpsiNRQCDGen.h"
using namespace std;

namespace DPGEN
{
  // global parameters
  const double pi = TMath::Pi();
  const double twopi = 2.*pi;
  const double sqrt2pi = TMath::Sqrt(twopi);
  
  // masses
  const double mp = 0.93827;
  const double mmu = 0.10566;
  const double mjpsi = 3.097;
  
  // 4-vectors
  const double ebeam = 120.;
  const TLorentzVector p_beam(0., 0., TMath::Sqrt(ebeam*ebeam - mp*mp), ebeam);
  const TLorentzVector p_target(0., 0., 0., mp);
  const TLorentzVector p_cms = p_beam + p_target;
  const TVector3 bv_cms = p_cms.BoostVector();
  const double s = p_cms.M2();
  const double sqrts = p_cms.M();
}


JpsiNRQCDGen::JpsiNRQCDGen(const std::string& name)
  : PHG4ParticleGeneratorBase(name)
  , _dir_data("$E1039_RESOURCE/generator/Jpsi_NRQCD")
  , _tgt(PROTON)
  , _pdf(CTEQ)
  , _ldme(SMRS_FIT)
  , _vertexGen(0)
  , _ineve(NULL)
  , _evt(0)
  , _mcevt(0)
  , _vec_dim(0)
  , _integral_node(0)
  , _dim_gen(0)
  , _pT0  (3.0)
  , _pTpow(1./(6. - 1.))
  , _n_gen_acc_evt(0)
  , _n_proc_evt(0)
  , _weight_sum(0)
  , _inte_lumi(0)
{
  ;
}

JpsiNRQCDGen::~JpsiNRQCDGen()
{
  if (_dim_gen  ) delete _dim_gen;
  if (_vertexGen) delete _vertexGen;
}

int JpsiNRQCDGen::Init(PHCompositeNode* topNode)
{
  _vertexGen = new SQPrimaryVertexGen();

  ostringstream oss;
  oss << _dir_data;
  if (_pdf == CTEQ) {
    if      (_tgt == PROTON  ) oss << "/CT18NLO/nrqcd_data1";
    else if (_tgt == DEUTERON) oss << "/CT18NLO/nrqcd_data2";
    else /* NITROGEN */        oss << "/nCTEQ15/nrqcd_data15";
  } else { // NNPDF
    if      (_tgt == PROTON  ) oss << "/NNPDF40NLO/nrqcd_data1";
    else if (_tgt == DEUTERON) oss << "/NNPDF40NLO/nrqcd_data2";
    else /* NITROGEN */        oss << "/nNNPDF30/nrqcd_data15";
  }
  oss << "_mem0_ldme";
  switch (_ldme) {
  case SMRS_REF   : oss << "1"; break;
  case SMRS_FIT   : oss << "5"; break;
  case GRV_REF    : oss << "2"; break;
  case GRV_FIT    : oss << "6"; break;
  case JAM_REF    : oss << "3"; break;
  case JAM_FIT    : oss << "7"; break;
  case xFitter_REF: oss << "4"; break;
  case xFitter_FIT: oss << "8"; break;
  }
  oss << "_jpsi.dat";
  char* fn_data = gSystem->ExpandPathName(oss.str().c_str());
  cout << "JpsiNRQCDGen::Init():  Read the cross-section data from " << fn_data << endl;
  
  ifstream ifs(fn_data);
  delete fn_data;
  double xF, v1, v2, v3, v4;
  while (ifs >> xF >> v1 >> v2 >> v3 >> v4) {
    if (v1 == 0) continue;
    int n_pt = _gr_xsec_total.GetN();
    _gr_xsec_total.SetPoint(n_pt, xF, v1);
    _gr_xsec_qqbar.SetPoint(n_pt, xF, v2);
    _gr_xsec_gg   .SetPoint(n_pt, xF, v3);
    _gr_xsec_qg   .SetPoint(n_pt, xF, v4);
  }
  ifs.close();
  if (_gr_xsec_total.GetN() == 0) {
    cout << "!!ERROR!!  Cannot read any data point.  Abort." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  
  return Fun4AllReturnCodes::EVENT_OK;
}

int JpsiNRQCDGen::InitRun(PHCompositeNode* topNode)
{ 
  gRandom->SetSeed(PHRandomSeed());

  _dim_gen = new SQDimuon_v1();
  
  PHNodeIterator iter( topNode );
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode) {
    cout << PHWHERE << "DST Node missing.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  _ineve = findNode::getClass<PHG4InEvent>(topNode,"PHG4INEVENT");
  if (!_ineve) {
    _ineve = new PHG4InEvent();
    dstNode->addNode(new PHIODataNode<PHObject>(_ineve, "PHG4INEVENT", "PHObject"));
  }
  _evt = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (! _evt) {
    _evt = new SQEvent_v2();
    dstNode->addNode(new PHIODataNode<PHObject>(_evt, "SQEvent", "PHObject"));
  }
  _mcevt = findNode::getClass<SQMCEvent>(topNode, "SQMCEvent");
  if (! _mcevt) {
    _mcevt = new SQMCEvent_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_mcevt, "SQMCEvent", "PHObject"));
  }
  _vec_dim = findNode::getClass<SQDimuonVector>(topNode, "SQTruthDimuonVector");
  if (! _vec_dim) {
    _vec_dim = new SQDimuonVector_v1();
    dstNode->addNode(new PHIODataNode<PHObject>(_vec_dim, "SQTruthDimuonVector", "PHObject"));
  }

  PHCompositeNode *runNode = dynamic_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!runNode) {
    cout << PHWHERE << "RUN Node missing.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  _integral_node = findNode::getClass<PHGenIntegral>(runNode, "PHGenIntegral");
  if (!_integral_node) {
    _integral_node = new PHGenIntegralv1("By JpsiNRQCDGen");
    runNode->addNode(new PHIODataNode<PHObject>(_integral_node, "PHGenIntegral", "PHObject"));
  } else {
    cout << PHWHERE << "PHGenIntegral Node exists.  Unexpected.  ABORTRUN." << endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }

  _vertexGen->InitRun(topNode);
  
  return 0;
}

int JpsiNRQCDGen::End(PHCompositeNode* topNode)
{
  recoConsts* rc = recoConsts::instance();
  rc->set_CharFlag  ("JPSINRQCDGEN_CHANNEL"    , "Jpsi");
  rc->set_IntFlag   ("JPSINRQCDGEN_EVENT_COUNT", _n_proc_evt);
  rc->set_DoubleFlag("JPSINRQCDGEN_xfMin"      , xfMin);
  rc->set_DoubleFlag("JPSINRQCDGEN_xfMax"      , xfMax);
  rc->set_DoubleFlag("JPSINRQCDGEN_pT0"        , _pT0  );
  rc->set_DoubleFlag("JPSINRQCDGEN_pTpow"      , _pTpow);

  return Fun4AllReturnCodes::EVENT_OK;
}

int JpsiNRQCDGen::process_event(PHCompositeNode* topNode)
{
  TVector3 vtx        = _vertexGen->generateVertex();
  Double_t pARatio    = _vertexGen->getPARatio();
  Double_t luminosity = _vertexGen->getLuminosity();

  int ret = generateJPsi(vtx, pARatio, luminosity);
  if (ret != 0) return Fun4AllReturnCodes::ABORTEVENT;

  _n_gen_acc_evt++;
  _n_proc_evt++;
  _weight_sum += _mcevt->get_weight();
  _inte_lumi  += luminosity;
  _integral_node->set_N_Generator_Accepted_Event(_n_gen_acc_evt);
  _integral_node->set_N_Processed_Event         (_n_proc_evt);
  _integral_node->set_Sum_Of_Weight             (_weight_sum);
  _integral_node->set_Integrated_Lumi           (_inte_lumi);

  _evt->set_run_id  (0);
  _evt->set_spill_id(0);
  _evt->set_event_id(_n_proc_evt);

  return Fun4AllReturnCodes::EVENT_OK; 
}

int JpsiNRQCDGen::generateJPsi(const TVector3& vtx, const double pARatio, double luminosity)
{
  double xF = gRandom->Uniform(0,1)*(xfMax - xfMin) + xfMin;
  if(!generateDimuon(DPGEN::mjpsi, xF)) return 1;
  InsertMuonPair(vtx);
  double xsec = _gr_xsec_total.Eval(xF);
  //const double BR = 0.0594; // Branching ratio of J/psi -> mumu.  Seen above Fig. 4 of PRD52_1307.
  double weight = xsec * luminosity * (xfMax - xfMin);
  InsertEventInfo(xsec, weight, vtx);
  return 0;
}

/**
 * Reference: G. Moreno et.al. Phys. Rev D43:2815-2836, 1991
 * The formula of pTmaxSq is discussed in DocDB 9292.
 */
bool JpsiNRQCDGen::generateDimuon(double mass, double xF)
{
  double pz = xF*(DPGEN::sqrts - mass*mass/DPGEN::sqrts)/2.;
    
  double pTmaxSq = (DPGEN::s / 4) * (1 - mass*mass/DPGEN::s)*(1 - mass*mass/DPGEN::s) * (1 - xF*xF);
  if(pTmaxSq < 0.) {
    cout << PHWHERE << "pTmaxSq < 0." << endl;
    return false;
  }
  
  double pTmax = sqrt(pTmaxSq);
  double pT = 10.;
  if (pTmax < 0.3) {
    pT = pTmax*sqrt(gRandom->Uniform(0,1));
  } else {
    while(pT > pTmax) pT = _pT0*TMath::Sqrt(1./TMath::Power(gRandom->Uniform(0,1), _pTpow) - 1.);
  }

  double phi = gRandom->Uniform(0,1)*DPGEN::twopi;
  double px = pT*TMath::Cos(phi);
  double py = pT*TMath::Sin(phi);
  
  //configure phase space generator
  TLorentzVector p_dimuon;
  p_dimuon.SetXYZM(px, py, pz, mass);
  p_dimuon.Boost(DPGEN::bv_cms);
  double masses[2] = {0.105658,0.105658};// Mass of muons
  phaseGen.SetDecay(p_dimuon, 2, masses);
  
  const int N_TRY = 1000000;
  for (int i_try = 0; i_try < N_TRY; i_try++) // Loop to generate a proper kinematics.
  {
    phaseGen.Generate();
    _dim_gen->set_mom_pos(*(phaseGen.GetDecay(0)));
    _dim_gen->set_mom_neg(*(phaseGen.GetDecay(1)));
    //double dim_mass, dim_pT, dim_x1, dim_x2, dim_xF;
    //UtilDimuon::CalcVar(_dim_gen, dim_mass, dim_pT, dim_x1, dim_x2, dim_xF);
    //if(dim_x1 < x1Min || dim_x1 > x1Max) continue;
    //if(dim_x2 < x2Min || dim_x2 > x2Max) continue;
    //double dim_costh, dim_phi;
    //UtilDimuon::Lab2CollinsSoper(_dim_gen, dim_costh, dim_phi);
    //if(dim_costh < cosThetaMin || dim_costh >cosThetaMax) continue;
    return true; // A proper dimuon is generated.
  }
  cout << PHWHERE << "No proper dimuon was generated.  Is your kinematic range reasonable?" << endl;
  return false;
}

/// Insert PHG4Particles objects of mu+ and mu- into "_ineve".
/**
 *  Note that "_ineve->AddVtx()" was called twice in generateJPsi() and generatePsip() 
 *  in the past versions since 2020-11-05.  But it seems not proper because a muon pair
 *   should share one vertex.
 */
void JpsiNRQCDGen::InsertMuonPair(const TVector3& vtx)
{
  int vtxindex = _ineve->AddVtx(vtx.X(),vtx.Y(),vtx.Z(),0.);

  PHG4Particle *particle_muNeg = new PHG4Particlev2();
  particle_muNeg->set_track_id(12);
  particle_muNeg->set_vtx_id(vtxindex);
  particle_muNeg->set_name("mu-");
  particle_muNeg->set_pid(13);
  particle_muNeg->set_px(_dim_gen->get_mom_neg().Px());
  particle_muNeg->set_py(_dim_gen->get_mom_neg().Py());
  particle_muNeg->set_pz(_dim_gen->get_mom_neg().Pz());
  particle_muNeg->set_e (_dim_gen->get_mom_neg().E ());
  _ineve->AddParticle(vtxindex, particle_muNeg);

  PHG4Particle *particle_muplus = new PHG4Particlev2();
  particle_muplus->set_track_id(2);
  particle_muplus->set_vtx_id(vtxindex);
  particle_muplus->set_name("mu+");
  particle_muplus->set_pid(-13);
  particle_muplus->set_px(_dim_gen->get_mom_pos().Px());
  particle_muplus->set_py(_dim_gen->get_mom_pos().Py());
  particle_muplus->set_pz(_dim_gen->get_mom_pos().Pz());
  particle_muplus->set_e (_dim_gen->get_mom_pos().E ());
  _ineve->AddParticle(vtxindex, particle_muplus);
}

/// Insert the event info into SQ interface objects.
/**
 * This function could be merged to InsertMuonPair().
 */
void JpsiNRQCDGen::InsertEventInfo(const double xsec, const double weight, const TVector3& vtx)
{
  static int dim_id = 0;

  _mcevt->set_process_id   (0);
  _mcevt->set_cross_section(xsec);
  _mcevt->set_weight       (weight);

  _vec_dim->clear();
  _dim_gen->set_dimuon_id   (++dim_id);
  _dim_gen->set_pos         (vtx);
  _dim_gen->set_mom         (_dim_gen->get_mom_pos() + _dim_gen->get_mom_neg());
  _dim_gen->set_track_id_pos( 2); // Given in InsertMuonPair().
  _dim_gen->set_track_id_neg(12); // Given in InsertMuonPair().
  _vec_dim->push_back(_dim_gen);
}
