#include "SQReco.h"

#include "KalmanFastTracking.h"
#include "EventReducer.h"

#include <phfield/PHFieldConfig_v3.h>
#include <phfield/PHFieldUtility.h>
#include <phgeom/PHGeomUtility.h>

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHIODataNode.h>
#include <phool/getClass.h>
#include <phool/recoConsts.h>
#include <phgeom/PHGeomTGeo.h>

#include <TFile.h>
#include <TTree.h>

#include <cstring>
#include <cmath>
#include <cfloat>
#include <cassert>
#include <stdexcept>
#include <limits>
#include <memory>
#include <fstream>
#include <exception>
#include <boost/lexical_cast.hpp>

#ifdef _DEBUG_ON
#  define LogDebug(exp) std::cout << "DEBUG: " << __FUNCTION__ <<": "<< __LINE__ << ": " << exp << std::endl
#else
#  define LogDebug(exp)
#endif

SQReco::SQReco(const std::string& name):
  SubsysReco(name),
  _input_type(SQReco::E1039),
  _fitter_type(SQReco::KFREF),
  _enable_eval(false),
  _eval_file_name("eval.root"),
  _eval_tree(nullptr),
  _tracklets(nullptr),
  _enable_eval_dst(false),
  _tracklet_vector(nullptr),
  _evt_reducer_opt(""),
  _fastfinder(nullptr),
  _eventReducer(nullptr),
  _enable_KF(true),
  _kfitter(nullptr),
  _gfitter(nullptr),
  _phfield(nullptr),
  _gfield(nullptr),
  _hit_container_type("Vector"),
  _event(0),
  _run_header(nullptr),
  _spill_map(nullptr),
  _event_header(nullptr),
  _hit_map(nullptr),
  _hit_vector(nullptr),
  _triggerhit_vector(nullptr),
  _rawEvent(nullptr),
  _recEvent(nullptr),
  _geom_file_name(""),
  _t_geo_manager(nullptr)
{
  p_jobOptsSvc = JobOptsSvc::instance();
  _eval_listIDs.clear();
}

SQReco::~SQReco()
{}

int SQReco::Init(PHCompositeNode* topNode) 
{
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::InitRun(PHCompositeNode* topNode) 
{
  if(is_eval_enabled())
  {
    InitEvalTree();
    ResetEvalVars();
  }

  int ret = MakeNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = GetNodes(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = InitField(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  ret = InitGeom(topNode);
  if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;

  //Init track finding
  _fastfinder = new KalmanFastTracking(_phfield, _t_geo_manager, false);
  _fastfinder->Verbosity(Verbosity());

  if(_evt_reducer_opt == "none")  //Meaning we disable the event reducer
  {
    _eventReducer = nullptr;
  }
  else if(_evt_reducer_opt == "") //Meaning we initialize the event reducer by opts
  {
    _evt_reducer_opt = "aoc";
    if(p_jobOptsSvc->m_enableTriggerMask) _evt_reducer_opt = _evt_reducer_opt + "t";
    if(p_jobOptsSvc->m_sagittaReducer)    _evt_reducer_opt = _evt_reducer_opt + "s";
    if(p_jobOptsSvc->m_updateAlignment)   _evt_reducer_opt = _evt_reducer_opt + "e";
    if(p_jobOptsSvc->m_hodomask)          _evt_reducer_opt = _evt_reducer_opt + "h";
    if(p_jobOptsSvc->m_mergeHodo)         _evt_reducer_opt = _evt_reducer_opt + "m";
    if(p_jobOptsSvc->m_realization)       _evt_reducer_opt = _evt_reducer_opt + "r";

    _eventReducer = new EventReducer(_evt_reducer_opt);
  }
  else
  {
    _eventReducer = new EventReducer(_evt_reducer_opt);
  }

  //Initialize the fitter
  if(_enable_KF)
  {
    if(_fitter_type == SQReco::LEGACY)
    {
      _kfitter = new KalmanFitter(_phfield, _t_geo_manager);
      _kfitter->setControlParameter(50, 0.001);
    }
    else 
    {
      _gfitter = new SQGenFit::GFFitter();
      if(_fitter_type == SQReco::KF)
      {
        _gfitter->init(_gfield, "KalmanFitter");
      }
      else if(_fitter_type == SQReco::KFREF)
      {
        _gfitter->init(_gfield, "KalmanFitterRefTrack");
      }
      else if(_fitter_type == SQReco::DAF)
      {
        _gfitter->init(_gfield, "DafSimple");
      }
      else if(_fitter_type == SQReco::DAFREF)
      {
        _gfitter->init(_gfield, "Daf");
      }

      //TODO: common settings for sqfitter
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::InitField(PHCompositeNode* topNode)
{
  if(Verbosity() > 1)
  {
    std::cout << "SQReco::InitField" << std::endl;
    if(!_enable_KF)
    {
      std::cout << " KF is disabled thus phfield is not needed. Skip InitField for SQReco." << std::endl;
      return Fun4AllReturnCodes::EVENT_OK;
    }
  }

  std::unique_ptr<PHFieldConfig> default_field_cfg(new PHFieldConfig_v3(p_jobOptsSvc->m_fMagFile, p_jobOptsSvc->m_kMagFile, recoConsts::instance()->get_DoubleFlag("FMAGSTR"), recoConsts::instance()->get_DoubleFlag("KMAGSTR"), 5.));
  _phfield = PHFieldUtility::GetFieldMapNode(default_field_cfg.get(), topNode, 0);

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT) 
  {
    std::cout << "PHField check: " << "-------" << std::endl;
    std::ofstream field_scan("field_scan.csv");
    _phfield->identify(field_scan);
    field_scan.close();
  }

  if(_fitter_type != SQReco::LEGACY) _gfield = new SQGenFit::GFField(_phfield);
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::InitGeom(PHCompositeNode* topNode)
{
  if(Verbosity() > 1) 
  {
    std::cout << "SQReco::InitGeom" << std::endl;
    if(!_enable_KF)
    {
      std::cout << " KF is disabled thus TGeo is not needed. Skip InitGeom for SQReco." << std::endl;
      return Fun4AllReturnCodes::EVENT_OK;
    }
  }

  PHGeomTGeo* dstGeom = PHGeomUtility::GetGeomTGeoNode(topNode, true); //hacky way to bypass PHGeoUtility's lack of exception throwing
  if(!dstGeom->isValid())
  {
    if(_geom_file_name == "") return Fun4AllReturnCodes::ABORTEVENT;

    if(Verbosity() > 1) std::cout << "SQReco::InitGeom - create geom from " << _geom_file_name << std::endl;
    int ret = PHGeomUtility::ImportGeomFile(topNode, _geom_file_name);
    if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  }
  else
  {
    if(Verbosity() > 1) std::cout << "SQReco::InitGeom - use geom from NodeTree." << std::endl;
  }

  _t_geo_manager = PHGeomUtility::GetTGeoManager(topNode);
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::updateHitInfo(SRawEvent* sraw_event) 
{
  for(Hit hit : sraw_event->getAllHits()) 
  {
    size_t idx = _m_hitID_idx[hit.index];
    SQHit* sq_hit = _hit_vector->at(idx);

    sq_hit->set_tdc_time(hit.tdcTime);
    sq_hit->set_drift_distance(hit.driftDistance);
    sq_hit->set_pos(hit.pos);
    sq_hit->set_in_time(hit.isInTime());
    sq_hit->set_hodo_mask(hit.isHodoMask());
    sq_hit->set_trigger_mask(hit.isTriggerMask());
  }

  return 0;
}

SRawEvent* SQReco::BuildSRawEvent() 
{
  //Needed for E1039 since we switched to a more generic interface
  //SRawEvent is still needed so that the code can be used for E906 as well
  SRawEvent* sraw_event = new SRawEvent();
  _m_hitID_idx.clear();
  _m_trghitID_idx.clear();

  int run_id   = 0;
  int spill_id = 0;
  int event_id = _event;
  if(_event_header) 
  {
    run_id   = _event_header->get_run_id();
    spill_id = _event_header->get_spill_id();
    event_id = _event_header->get_event_id();
  }
  sraw_event->setEventInfo(run_id, spill_id, event_id);

  //Trigger setting - either from trigger emulation or TS, default to 0
  int triggers[10];
  for(int i = SQEvent::NIM1; i <= SQEvent::MATRIX5; ++i)
  {
    if(_event_header)
      triggers[i] = _event_header->get_trigger(static_cast<SQEvent::TriggerMask>(i));
    else
      triggers[i] = 0;
  }
  sraw_event->setTriggerBits(triggers);

  //Get target position
  int targetPos = 0;
  if(_spill_map) 
  {
    SQSpill* spill = _spill_map->get(spill_id);
    if(spill) 
    {
      targetPos = spill->get_target_pos();
    }
  }
  sraw_event->setTargetPos(1);

  //Get beam information - QIE -- not implemented yet

  //Get trigger hits - TriggerHit
  if(_triggerhit_vector) 
  {
    for(size_t idx = 0; idx < _triggerhit_vector->size(); ++idx) 
    {
      SQHit* sq_hit = _triggerhit_vector->at(idx);
      _m_trghitID_idx[sq_hit->get_hit_id()] = idx;

      Hit h;
      h.index = sq_hit->get_hit_id();
      h.detectorID = sq_hit->get_detector_id();
      h.elementID = sq_hit->get_element_id();
      h.tdcTime = sq_hit->get_tdc_time();
      h.driftDistance = fabs(sq_hit->get_drift_distance()); //MC L-R info removed here
      h.pos = sq_hit->get_pos();

      if(sq_hit->is_in_time()) h.setInTime();
      sraw_event->insertTriggerHit(h);
    }
  }

  for(size_t idx = 0; idx < _hit_vector->size(); ++idx) 
  {
    SQHit* sq_hit = _hit_vector->at(idx);

    Hit h;
    h.index = sq_hit->get_hit_id();
    h.detectorID = sq_hit->get_detector_id();
    h.elementID = sq_hit->get_element_id();
    h.tdcTime = sq_hit->get_tdc_time();
    h.driftDistance = fabs(sq_hit->get_drift_distance()); //MC L-R info removed here
    h.pos = sq_hit->get_pos();

    if(sq_hit->is_in_time()) h.setInTime();
    sraw_event->insertHit(h);

    /* We should not need the following code, since all these logic are done in
    // inside eventreducer
    //TODO calibration
    if(p_geomSvc->isCalibrationLoaded())
    { 
      if((h.detectorID >= 1 && h.detectorID <= nChamberPlanes) || (h.detectorID >= nChamberPlanes+nHodoPlanes+1))
      {
        h.setInTime(p_geomSvc->isInTime(h.detectorID, h.tdcTime));
        if(h.isInTime()) h.driftDistance = p_geomSvc->getDriftDistance(h.detectorID, h.tdcTime);
      }
    }

    // FIXME just for the meeting, figure this out fast!
    if(!_triggerhit_vector and h.detectorID >= 31 and h.detectorID <= 46) {
      sraw_event->insertTriggerHit(h);
    }
    */
  }

  sraw_event->reIndex(true);
  return sraw_event;
}

int SQReco::process_event(PHCompositeNode* topNode) 
{
  LogDebug("Entering SQReco::process_event: " << _event);

  if(is_eval_enabled()) ResetEvalVars();
  if(_input_type == SQReco::E1039)
  {
    if(!_event_header) 
    {
      if(Verbosity() > 2) LogDebug("!_event_header");
      //return Fun4AllReturnCodes::ABORTRUN;
    }

    if(!_spill_map) 
    {
      if(Verbosity() > 2) LogDebug("!_spill_map");
      //return Fun4AllReturnCodes::ABORTRUN;
    }

    if(!_hit_vector) 
    {
      if(Verbosity() > 2) LogDebug("!_hit_vector");
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  std::unique_ptr<SRawEvent> up_raw_event;
  if(_input_type == SQReco::E1039) 
  {
    up_raw_event = std::unique_ptr<SRawEvent>(BuildSRawEvent());
    _rawEvent = up_raw_event.get();
  }
  _recEvent->setRawEvent(_rawEvent);

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT) 
  {
    LogInfo("SRawEvent before the Reducer");
    _rawEvent->identify();
  }

  if(_eventReducer != nullptr) 
  {
    _eventReducer->reduceEvent(_rawEvent);
    if(_input_type == SQReco::E1039) updateHitInfo(_rawEvent);
  }

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT) 
  {
    LogInfo("SRawEvent after the Reducer");
    _rawEvent->identify();
  }

  int finderstatus = _fastfinder->setRawEvent(_rawEvent);
  _recEvent->setRecStatus(finderstatus);
  if(Verbosity() >= Fun4AllBase::VERBOSITY_A_LOT) _fastfinder->printTimers();

  int nTracklets = 0;
  int nFittedTracks = 0;
  std::list<Tracklet>& rec_tracklets = _fastfinder->getFinalTracklets();
  for(auto iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter)
  {
    iter->calcChisq();
    if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT) iter->print();

    bool fitOK = false;
    if(_enable_KF)
    {
      if(_fitter_type == SQReco::LEGACY)
        fitOK = fitTrackCand(*iter, _kfitter);
      else
        fitOK = fitTrackCand(*iter, _gfitter);
    }

    if(!fitOK)
    {
      SRecTrack recTrack = iter->getSRecTrack(_enable_KF && (_fitter_type == SQReco::LEGACY));
      recTrack.setKalmanStatus(-1);
      _recEvent->insertTrack(recTrack);
    }
    else
    {
      ++nFittedTracks;
    }

    if(is_eval_enabled()) new((*_tracklets)[nTracklets]) Tracklet(*iter);
    if(is_eval_dst_enabled()) _tracklet_vector->push_back(&(*iter));
    ++nTracklets;
  }
  LogDebug("Leaving SQReco::process_event: " << _event << ", finder status " << finderstatus << ", " << nTracklets << " track candidates, " << nFittedTracks << " fitted tracks");

  //add additional eval information if applicable
  for(unsigned int i = 0; i < _eval_listIDs.size(); ++i)
  {
    std::list<Tracklet>& eval_tracklets = _fastfinder->getTrackletList(_eval_listIDs[i]);
    for(auto iter = eval_tracklets.begin(); iter != eval_tracklets.end(); ++iter)
    {
      new((*_tracklets)[nTracklets]) Tracklet(*iter);
      ++nTracklets;
    }
  }
  if(is_eval_enabled() && nTracklets > 0) _eval_tree->Fill();

  ++_event;
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::End(PHCompositeNode* topNode) 
{
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) std::cout << "SQReco::End" << std::endl;
  if(is_eval_enabled())
  {
    PHTFileServer::get().cd(_eval_file_name.Data());
    _eval_tree->Write();
  }

  delete _fastfinder;
  if(_gfitter != nullptr) delete _gfitter;

  return Fun4AllReturnCodes::EVENT_OK;
}

bool SQReco::fitTrackCand(Tracklet& tracklet, KalmanFitter* fitter)
{
  KalmanTrack kmtrk;
  kmtrk.setTracklet(tracklet);

  if(kmtrk.getNodeList().empty()) 
  {
    LogDebug("kmtrk nodelist empty");
    return false;
  }

  if(_kfitter->processOneTrack(kmtrk) == 0)
  {
    LogDebug("kFitter failed to converge");
    return false;
  }

  if(!kmtrk.isValid()) 
  {
    LogDebug("kmtrk quality cut failed");
    return false;
  }

  SRecTrack strack = kmtrk.getSRecTrack();

  //Set trigger road ID
  TriggerRoad road(tracklet);
  strack.setTriggerRoad(road.getRoadID());

  //Set prop tube slopes
  strack.setNHitsInPT(tracklet.seg_x.getNHits(), tracklet.seg_y.getNHits());
  strack.setPTSlope(tracklet.seg_x.a, tracklet.seg_y.a);

  _recEvent->insertTrack(strack);
  return true;
}

bool SQReco::fitTrackCand(Tracklet& tracklet, SQGenFit::GFFitter* fitter)
{
  SQGenFit::GFTrack gftrk;
  gftrk.setTracklet(tracklet);

  int fitOK = _gfitter->processTrack(gftrk);
  if(fitOK != 0)
  {
    LogDebug("gFitter failed to converge.");
    return false;
  }

  if(Verbosity() > Fun4AllBase::VERBOSITY_A_LOT)
  {
    gftrk.postFitUpdate();
    gftrk.print(2);
  }

  //TODO: A gtrack quality cut?

  SRecTrack strack = gftrk.getSRecTrack();

  //Set trigger road ID
  TriggerRoad road(tracklet);
  strack.setTriggerRoad(road.getRoadID());

  //Set prop tube slopes
  strack.setNHitsInPT(tracklet.seg_x.getNHits(), tracklet.seg_y.getNHits());
  strack.setPTSlope(tracklet.seg_x.a, tracklet.seg_y.a);

  _recEvent->insertTrack(strack);
  return true;
}

int SQReco::InitEvalTree() 
{
  PHTFileServer::get().open(_eval_file_name.Data(), "RECREATE");

  _tracklets = new TClonesArray("Tracklet");

  _eval_tree = new TTree("eval", "eval");
  _eval_tree->Branch("eventID", &_event, "eventID/I");
  _eval_tree->Branch("tracklets", &_tracklets, 256000, 99);
  _tracklets->BypassStreamer();

  return 0;
}

int SQReco::ResetEvalVars() 
{
  _tracklets->Clear();
  return 0;
}

int SQReco::MakeNodes(PHCompositeNode* topNode) 
{
  PHNodeIterator iter(topNode);
  PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if(!eventNode) 
  {
    LogInfo("No DST node, create one");
    eventNode = new PHCompositeNode("DST");
    topNode->addNode(eventNode);
  }

  _recEvent = new SRecEvent();
  PHIODataNode<PHObject>* recEventNode = new PHIODataNode<PHObject>(_recEvent, "SRecEvent", "PHObject");
  eventNode->addNode(recEventNode);
  if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/SRecEvent Added");

  if(_enable_eval_dst)
  {
    _tracklet_vector = new TrackletVector();
    _tracklet_vector->SplitLevel(99);
    PHIODataNode<PHObject>* trackletVecNode = new PHIODataNode<PHObject>(_tracklet_vector, "TrackletVector", "PHObject");
    eventNode->addNode(trackletVecNode);
    if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/TrackletVector Added");
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::GetNodes(PHCompositeNode* topNode) 
{
  if(_input_type == SQReco::E1039)
  {
    _run_header = findNode::getClass<SQRun>(topNode, "SQRun");
    if(!_run_header) 
    {
      if(Verbosity() > 2) LogDebug("!_run_header");
      //return Fun4AllReturnCodes::ABORTEVENT;
    }

    _spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
    if(!_spill_map) 
    {
      if(Verbosity() > 2) LogDebug("!_spill_map");
      //return Fun4AllReturnCodes::ABORTEVENT;
    }

    _event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
    if(!_event_header) 
    {
      if(Verbosity() > 2) LogDebug("!_event_header");
      //return Fun4AllReturnCodes::ABORTEVENT;
    }

    if(_hit_container_type.find("Map") != std::string::npos) 
    {
      _hit_map = findNode::getClass<SQHitMap>(topNode, "SQHitMap");
      if(!_hit_map) 
      {
        LogDebug("!_hit_map");
        return Fun4AllReturnCodes::ABORTEVENT;
      }
    }

    if(_hit_container_type.find("Vector") != std::string::npos) 
    {
      _hit_vector = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
      if(!_hit_vector) 
      {
        LogDebug("!_hit_vector");
        return Fun4AllReturnCodes::ABORTEVENT;
      }

      _triggerhit_vector = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
      if(!_triggerhit_vector) 
      {
        if(Verbosity() > 2) LogDebug("!_triggerhit_vector");
        //return Fun4AllReturnCodes::ABORTEVENT;
      }
    }
  }
  else
  {
    _rawEvent = findNode::getClass<SRawEvent>(topNode, "SRawEvent");
    if(_rawEvent) 
    {
      if(Verbosity() > 0) LogInfo("Using SRawEvent as input for E906-like data input");
    }
  }

  _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent");
  if(!_recEvent) 
  {
    if(Verbosity() > 2) LogDebug("!_recEvent");
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  return Fun4AllReturnCodes::EVENT_OK;
}
