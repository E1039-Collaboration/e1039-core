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
class JobOptsSvc;

class SQRun;
class SQSpillMap;

class SQEvent;
class SQHitMap;
class SQHitVector;

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

  int Init(PHCompositeNode* topNode);
  int InitRun(PHCompositeNode* topNode);
  int process_event(PHCompositeNode* topNode);
  int End(PHCompositeNode* topNode);

  void setInputTy(SQReco::INPUT_TYPE input_ty) { _input_type = input_ty; }
  void setFitterTy(SQReco::FITTER_TYPE fitter_ty) { _fitter_type = fitter_ty; }

  const std::string& get_hit_container_choice() const { return _hit_container_type; }
  void set_hit_container_choice(const std::string& hitContainerChoice) { _hit_container_type = hitContainerChoice; }

  const TString& get_eval_file_name() const { return _eval_file_name; }
  void set_eval_file_name(const TString& evalFileName) { _eval_file_name = evalFileName; }

  const std::string& get_geom_file_name() const { return _geom_file_name; }
  void set_geom_file_name(const std::string& geomFileName) { _geom_file_name = geomFileName; }

  bool is_KF_enabled() const { return _enable_KF; }
  void set_enable_KF(bool enable) { _enable_KF = enable; }

  bool is_eval_enabled() const { return _enable_eval; }
  void set_enable_eval(bool enable) { _enable_eval = enable; }
  bool is_eval_dst_enabled() const { return _enable_eval_dst; }
  void set_enable_eval_dst(bool enable) { _enable_eval_dst = enable; }
  void add_eval_list(int listID) { _eval_listIDs.push_back(listID); }

  const TString& get_evt_reducer_opt() const { return _evt_reducer_opt; }
  void set_evt_reducer_opt(const TString& opt) { _evt_reducer_opt = opt; }

  bool fitTrackCand(Tracklet& tracklet, KalmanFitter* fitter);
  bool fitTrackCand(Tracklet& tracklet, SQGenFit::GFFitter* fitter);

private:

  int InitField(PHCompositeNode* topNode);
  int InitGeom(PHCompositeNode* topNode);
  int MakeNodes(PHCompositeNode* topNode);
  int GetNodes(PHCompositeNode* topNode);

  int InitEvalTree();
  int ResetEvalVars();

  SRawEvent* BuildSRawEvent();
  int updateHitInfo(SRawEvent* sraw_event);

  SQReco::INPUT_TYPE  _input_type;
  SQReco::FITTER_TYPE _fitter_type;

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

  JobOptsSvc* p_jobOptsSvc;

  std::string _hit_container_type;
  size_t _event;

  SQRun*      _run_header;
  SQSpillMap* _spill_map;

  SQEvent*     _event_header;
  SQHitMap*    _hit_map;
  SQHitVector* _hit_vector;
  SQHitVector* _triggerhit_vector;

  //map from the unique hitID to the index in the SQHitVector
  std::map<int, size_t> _m_hitID_idx;
  std::map<int, size_t> _m_trghitID_idx;

  SRawEvent* _rawEvent;
  SRecEvent* _recEvent;

  std::string  _geom_file_name;
  TGeoManager* _t_geo_manager;
};

#endif
