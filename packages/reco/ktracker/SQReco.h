/*
 * Entry class for all SpinQuest reconstruction
 */

#ifndef _SQRECO_H
#define _SQRECO_H

#include <fun4all/SubsysReco.h>

#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

#include <TString.h>

#include "GFFitter.h"

class PHField;

class Tracklet;
class KalmanFastTracking;
class KalmanFitter;
class EventReducer;
class SRawEvent;
class SRecEvent;

class SQRun;
class SQSpillMap;

class SQEvent;
class SQHitMap;
class SQHitVector;
class SQTrackVector;

class TFile;
class TTree;
class TGeoManager;
class TClonesArray;

class SQReco: public SubsysReco 
{
public:
  enum INPUT_TYPE  {E906, E1039};
  enum FITTER_TYPE {LEGACY, KF, KFREF, DAF, DAFREF};

  SQReco(const std::string& name = "SQReco");
  virtual ~SQReco();

  virtual int Init(PHCompositeNode* topNode);
  virtual int InitRun(PHCompositeNode* topNode);
  virtual int process_event(PHCompositeNode* topNode);
  virtual int End(PHCompositeNode* topNode);

  void setInputTy(SQReco::INPUT_TYPE input_ty) { _input_type = input_ty; }
  void setFitterTy(SQReco::FITTER_TYPE fitter_ty) { _fitter_type = fitter_ty; }

  const TString& get_eval_file_name() const { return _eval_file_name; }
  void set_eval_file_name(const TString& evalFileName) { _eval_file_name = evalFileName; }

  bool use_geom_io_node() const  { return _use_geom_io_node; }
  void use_geom_io_node(const bool val) { _use_geom_io_node = val; }

  const std::string& get_geom_file_name() const { return _geom_file_name; }
  void set_geom_file_name(const std::string& geomFileName) { _geom_file_name = geomFileName; }

  bool is_KF_enabled() const { return _enable_KF; }
  void set_enable_KF(bool enable) { _enable_KF = enable; }

  /// See `KalmanFastTracking::setOutputListID()`.
  void set_output_list_index(const int idx) { _output_list_idx = idx; }

  bool is_eval_enabled() const { return _enable_eval; }
  void set_enable_eval(bool enable) { _enable_eval = enable; }
  bool is_eval_dst_enabled() const { return _enable_eval_dst; }
  void set_enable_eval_dst(bool enable) { _enable_eval_dst = enable; }
  void add_eval_list(int listID);

  const TString& get_evt_reducer_opt() const { return _evt_reducer_opt; }
  void set_evt_reducer_opt(const TString& opt) { _evt_reducer_opt = opt; }

  void set_legacy_rec_container(const bool b = true) { _legacy_rec_container = b; } 

protected:

  virtual int InitField(PHCompositeNode* topNode);
  virtual int InitGeom(PHCompositeNode* topNode);
  virtual int InitFastTracking();
  virtual int MakeNodes(PHCompositeNode* topNode);
  virtual int GetNodes(PHCompositeNode* topNode);

  int InitEvalTree();
  int ResetEvalVars();

  void ProcessEventPrep();
  void ProcessEventFinish();
  SRawEvent* BuildSRawEvent();
  int updateHitInfo(SRawEvent* sraw_event);

  bool fitTrackCand(Tracklet& tracklet, KalmanFitter* fitter);
  bool fitTrackCand(Tracklet& tracklet, SQGenFit::GFFitter* fitter);

  void fillRecTrack(SRecTrack& recTrack);

  SQReco::INPUT_TYPE  _input_type;
  SQReco::FITTER_TYPE _fitter_type;

  int _output_list_idx;

  bool _enable_eval;
  TString _eval_file_name;
  TTree*  _eval_tree;
  TClonesArray* _tracklets;
  std::vector<int> _eval_listIDs;

  bool _enable_eval_dst;
  TrackletVector* _tracklet_vector;

  TString _evt_reducer_opt;
  KalmanFastTracking* _fastfinder;
  EventReducer*       _eventReducer;

  bool _enable_KF;
  KalmanFitter*       _kfitter;
  SQGenFit::GFFitter* _gfitter;

  PHField* _phfield;
  SQGenFit::GFField* _gfield;

  recoConsts* rc;

  size_t _event;

  SQRun*      _run_header;
  SQSpillMap* _spill_map;

  SQEvent*     _event_header;
  SQHitVector* _hit_vector;
  SQHitVector* _triggerhit_vector;

  //map from the unique hitID to the index in the SQHitVector
  std::map<int, size_t> _m_hitID_idx;
  std::map<int, size_t> _m_trghitID_idx;

  bool _legacy_rec_container;
  SRawEvent* _rawEvent;
  SRecEvent* _recEvent;
  SQTrackVector* _recTrackVec;

  bool _use_geom_io_node;
  std::string  _geom_file_name;
  TGeoManager* _t_geo_manager;
};

#endif
