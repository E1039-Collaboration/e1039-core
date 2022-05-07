#include "SQReco.h"

#include "KalmanFastTracking.h"
#include "EventReducer.h"
#include "UtilSRawEvent.h"

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
#include <interface_main/SQTrackVector_v1.h>

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
  _output_list_idx(0),
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
  _event(0),
  _run_header(nullptr),
  _spill_map(nullptr),
  _event_header(nullptr),
  _hit_vector(nullptr),
  _triggerhit_vector(nullptr),
  _legacy_rec_container(true),
  _rawEvent(nullptr),
  _recEvent(nullptr),
  _recTrackVec(nullptr),
  _use_geom_io_node(false),
  _geom_file_name(""),
  _t_geo_manager(nullptr)
{
  rc = recoConsts::instance();
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

  InitFastTracking();

  if(_evt_reducer_opt == "none")  //Meaning we disable the event reducer
  {
    _eventReducer = nullptr;
  }
  else if(_evt_reducer_opt == "") //Meaning we initialize the event reducer by opts
  {
    _evt_reducer_opt = rc->get_CharFlag("EventReduceOpts");

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
        _gfitter->init(_gfield, "DafRef");
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

  std::unique_ptr<PHFieldConfig> default_field_cfg(new PHFieldConfig_v3(rc->get_CharFlag("fMagFile"), rc->get_CharFlag("kMagFile"), rc->get_DoubleFlag("FMAGSTR"), rc->get_DoubleFlag("KMAGSTR"), 5.));
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

  if (_geom_file_name != "")
  {
    if(Verbosity() > 1) std::cout << "SQReco::InitGeom - create geom from " << _geom_file_name << std::endl;
    if (_use_geom_io_node)
    {
      std::cout << "SQReco::InitGeom - Both 'geom_file_name' and 'use_geom_io_node' are active.  Use only one." << std::endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
    int ret = PHGeomUtility::ImportGeomFile(topNode, _geom_file_name);
    if(ret != Fun4AllReturnCodes::EVENT_OK) return ret;
  }
  else if (_use_geom_io_node)
  {
    if(Verbosity() > 1) std::cout << "SQReco::InitGeom - use geom from RUN node tree." << std::endl;
    PHGeomTGeo* node = PHGeomUtility::LoadFromIONode(topNode);
    if (! node)
    {
      std::cout << "SQReco::InitGeom - Failed at loading the GEOMETRY_IO node." << std::endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }
  else
  {
    if(Verbosity() > 1) std::cout << "SQReco::InitGeom - use geom from PAR node tree." << std::endl;
    PHGeomTGeo* dstGeom = PHGeomUtility::GetGeomTGeoNode(topNode, true); //hacky way to bypass PHGeoUtility's lack of exception throwing
    if(!dstGeom->isValid())
    {
      std::cout << "SQReco::InitGeom - Failed at loading the GEOMETRY node." << std::endl;
      return Fun4AllReturnCodes::ABORTEVENT;
    }
  }

  _t_geo_manager = PHGeomUtility::GetTGeoManager(topNode);
  return Fun4AllReturnCodes::EVENT_OK;
}

int SQReco::InitFastTracking()
{
  _fastfinder = new KalmanFastTracking(_phfield, _t_geo_manager, false);

  _fastfinder->Verbosity(Verbosity());

  if (_output_list_idx > 0) _fastfinder->setOutputListIndex(_output_list_idx);
  return 0;
}

void SQReco::ProcessEventPrep()
{
  if(is_eval_enabled()) ResetEvalVars();
  if(is_eval_dst_enabled()) _tracklet_vector->clear();

  if(_input_type == SQReco::E1039) _rawEvent = BuildSRawEvent();

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
}

void SQReco::ProcessEventFinish()
{
  if(_input_type == SQReco::E1039) {
    delete _rawEvent;
    _rawEvent = 0;
  }
  ++_event;
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

  if(!UtilSRawEvent::SetEvent(sraw_event, _event_header))
  {
    sraw_event->setEventInfo(0, 0, _event); // overwrite event ID
  }

  //Get target position
  if(_spill_map)
  {
    UtilSRawEvent::SetSpill(sraw_event, _spill_map->get( sraw_event->getSpillID() ));
  }

  //Get beam information - QIE -- not implemented yet

  //Get trigger hits - TriggerHit
  UtilSRawEvent::SetTriggerHit(sraw_event, _triggerhit_vector, &_m_trghitID_idx);
  UtilSRawEvent::SetHit       (sraw_event, _hit_vector       , &_m_hitID_idx);

  return sraw_event;
}

int SQReco::process_event(PHCompositeNode* topNode) 
{
  LogDebug("Entering SQReco::process_event: " << _event);

  ProcessEventPrep();

  int finderstatus = _fastfinder->setRawEvent(_rawEvent);
  if(_legacy_rec_container) 
  {
    _recEvent->setRawEvent(_rawEvent);
    _recEvent->setRecStatus(finderstatus);
  }
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
      
      fillRecTrack(recTrack);
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
  if(is_eval_enabled() || is_eval_dst_enabled())
  {
    for(unsigned int i = 0; i < _eval_listIDs.size(); ++i)
    {
      std::list<Tracklet>& eval_tracklets = _fastfinder->getTrackletList(_eval_listIDs[i]);
      for(auto iter = eval_tracklets.begin(); iter != eval_tracklets.end(); ++iter)
      {
        if(is_eval_enabled())
        {
          new((*_tracklets)[nTracklets]) Tracklet(*iter);
          ++nTracklets;
        }

        if(is_eval_dst_enabled()) _tracklet_vector->push_back(&(*iter));
      }
    }
  }
  
  if(is_eval_enabled() && nTracklets > 0) _eval_tree->Fill();

  ProcessEventFinish();

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

  _kfitter->updateTrack(kmtrk);//update after fitting

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
  strack.setKalmanStatus(1);
  fillRecTrack(strack);
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

  fillRecTrack(strack);
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

void SQReco::fillRecTrack(SRecTrack& recTrack)
{
  if(_legacy_rec_container)
    _recEvent->insertTrack(recTrack);
  else
    _recTrackVec->push_back(&recTrack);
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

  if(_legacy_rec_container)
  {
    _recEvent = findNode::getClass<SRecEvent>(topNode, "SRecEvent"); // Could exist when the tracking is re-done.
    if(!_recEvent) {
      _recEvent = new SRecEvent();
      eventNode->addNode(new PHIODataNode<PHObject>(_recEvent, "SRecEvent", "PHObject"));
      if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/SRecEvent Added");
    }
  }
  else
  {
    _recTrackVec = new SQTrackVector_v1();
    PHIODataNode<PHObject>* recEventNode = new PHIODataNode<PHObject>(_recTrackVec, "SQRecTrackVector", "PHObject");
    eventNode->addNode(recEventNode);
    if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/SQRecTrackVector Added");
  }

  if(_enable_eval_dst)
  {
    _tracklet_vector = findNode::getClass<TrackletVector>(topNode, "TrackletVector"); // Could exist when the tracking is re-done.
    if(!_tracklet_vector) {
      _tracklet_vector = new TrackletVector();
      _tracklet_vector->SplitLevel(99);
      eventNode->addNode(new PHIODataNode<PHObject>(_tracklet_vector, "TrackletVector", "PHObject"));
      if(Verbosity() >= Fun4AllBase::VERBOSITY_SOME) LogInfo("DST/TrackletVector Added");
    }
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
  else
  {
    _rawEvent = findNode::getClass<SRawEvent>(topNode, "SRawEvent");
    if(_rawEvent) 
    {
      if(Verbosity() > 0) LogInfo("Using SRawEvent as input for E906-like data input");
    }
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void SQReco::add_eval_list(int listID)
{
  if(std::find(_eval_listIDs.begin(), _eval_listIDs.end(), listID) == _eval_listIDs.end())
  {
    _eval_listIDs.push_back(listID);
  }
}
