/*
 * KalmanFastTrackingWrapper.h
 *
 *  Created on: Oct 29, 2017
 *      Author: yuhw@nmsu.edu
 */

#ifndef _H_KalmanFastTrackingWrapper_H_
#define _H_KalmanFastTrackingWrapper_H_

#include <TSQLServer.h>
#include <TSQLResult.h>
#include <TSQLRow.h>

#include <fun4all/SubsysReco.h>

#include <vector>
#include <string>
#include <iostream>
#include <list>
#include <map>

class KalmanFastTracking;
//class KalmanDSTrk;
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

class KalmanFastTrackingWrapper: public SubsysReco {

public:
  enum INPUPT_TYPE {E1039, E906};

  KalmanFastTrackingWrapper(const std::string &name = "KalmanFastTrackingWrapper");
  virtual ~KalmanFastTrackingWrapper() {
    //delete fastfinder;
    //delete eventReducer;
  }

  int Init(PHCompositeNode *topNode);
  int InitRun(PHCompositeNode *topNode);
  int process_event(PHCompositeNode *topNode);
  int End(PHCompositeNode *topNode);

  int InitEvalTree();
  int ResetEvalVars();

  const std::string& get_hit_container_choice() const {
    return _hit_container_type;
  }

  void set_hit_container_choice(const std::string& hitContainerChoice) {
    _hit_container_type = hitContainerChoice;
  }

  const std::string& get_out_name() const {
    return _out_name;
  }

  void set_out_name(const std::string& outName) {
    _out_name = outName;
  }

  const std::string& get_geom_file_name() const {
    return _geom_file_name;
  }

  void set_geom_file_name(const std::string& geomFileName) {
    _geom_file_name = geomFileName;
  }

  bool is_enable_KF() const {
    return _enable_KF;
  }

  void set_enable_KF(bool enableKf) {
    _enable_KF = enableKf;
  }

  bool is_enable_event_reducer() const {
    return _enable_event_reducer;
  }

  void set_enable_event_reducer(bool enableEventReducer) {
    _enable_event_reducer = enableEventReducer;
  }

  /*
  int get_DS_level() const {
    return _DS_level;
  }

  void set_DS_level(int DS) {
    _DS_level = DS;
  }

  const std::string& get_pattern_db_name() const {
    return _pattern_db_name;
  }

  void set_pattern_db_name(const std::string& patternDbName) {
    _pattern_db_name = patternDbName;
  }

  const std::string& get_sim_db_name() const {
    return _sim_db_name;
  }

  void set_sim_db_name(const std::string& simDbName) {
    _sim_db_name = simDbName;
  }
  */

private:

  int InitField(PHCompositeNode *topNode);

  int InitGeom(PHCompositeNode *topNode);

  int MakeNodes(PHCompositeNode *topNode);

  int GetNodes(PHCompositeNode *topNode);

  int ReMaskHits(SRawEvent *sraw_event);

  SRawEvent* BuildSRawEvent();

  KalmanFastTrackingWrapper::INPUPT_TYPE _input_type;
  bool _enable_KF;
  bool _enable_event_reducer;
  
  int _DS_level;
  std::string _sim_db_name;
  std::string _pattern_db_name;



  KalmanFastTracking* fastfinder;
  //KalmanDSTrk* fastfinder;
  EventReducer* eventReducer;

  std::string _hit_container_type;

  size_t _event;

  SQRun* _run_header;
  SQSpillMap * _spill_map;

  SQEvent * _event_header;
  SQHitMap *_hit_map;
  SQHitVector *_hit_vector;
  SQHitVector *_triggerhit_vector;

  std::string _out_name;
  TTree* _tout;

  SRawEvent* _rawEvent;
  SRecEvent* _recEvent;

  JobOptsSvc* p_jobOptsSvc;
  std::string _geom_file_name;
  TGeoManager* _t_geo_manager;
};

#endif /* _H_KalmanFastTrackingWrapper_H_ */
