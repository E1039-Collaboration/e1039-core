#include "Fun4AllEVIOInputManager.h"

#include "MainDaqParser.h"
#include "CodaInputManager.h"
#include "DecoData.h"

//#include <event/EVIO_Event.h>
#include <interface_main/SQParamDeco_v1.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v2.h>
#include <interface_main/SQHardEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v2.h>
#include <interface_main/SQSpillMap_v1.h>
#include <interface_main/SQHardSpill_v1.h>
#include <interface_main/SQIntMap_v1.h>
#include <interface_main/SQStringMap.h>
#include <interface_main/SQScaler_v1.h>
#include <interface_main/SQSlowCont_v1.h>
 
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllSyncManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllUtils.h>

#include <ffaobjects/RunHeader.h>
#include <ffaobjects/SyncObjectv2.h>

#include <phool/getClass.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHDataNode.h>
#include <phool/recoConsts.h>
#include <phool/PHTimer2.h>
#include <cstdlib>
#include <iomanip>
#include <memory>

//#include <boost/tokenizer.hpp>
//#include <boost/foreach.hpp>
//#include <boost/lexical_cast.hpp>

using namespace std;

const std::vector<std::string> Fun4AllEVIOInputManager::LIST_TIMERS = {
  "run_open"          , 
  "run_get_node"      , 
  "run_get_event"     , 
  "run_set_param"     , 
  "run_set_run_data"  , 
  "run_set_spill_data", 
  "run_set_event_data", 
  "run_set_hit_data"
};

Fun4AllEVIOInputManager::Fun4AllEVIOInputManager(const string &name, const string &topnodename) :
 Fun4AllInputManager(name, ""),
 segment(-999),
 isopen(0),
 events_total(0),
 events_thisfile(0),
 topNodeName(topnodename),
 //evt(NULL),
 //save_evt(NULL),
 parser(new MainDaqParser())
 //coda(NULL)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  topNode = se->topNode(topNodeName.c_str());
  PHNodeIterator iter(topNode);

  ////////////////////////////////////////////////////////////////
  PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!runNode) {
    runNode = new PHCompositeNode("RUN");
    topNode->addNode(runNode);
  }

  PHIODataNode<PHObject>* runHeaderNode = new PHIODataNode<PHObject>(new SQRun_v1(), "SQRun", "PHObject");
  runNode->addNode(runHeaderNode);
  
  PHIODataNode<PHObject>* spillNode = new PHIODataNode<PHObject>(new SQSpillMap_v1(), "SQSpillMap", "PHObject");
  runNode->addNode(spillNode);

  PHIODataNode<PHObject>* hardSpillNode = new PHIODataNode<PHObject>(new SQIntMap_v1(), "SQHardSpillMap", "PHObject");
  runNode->addNode(hardSpillNode);

  PHIODataNode<PHObject>* paramDecoNode = new PHIODataNode<PHObject>(new SQParamDeco_v1(), "SQParamDeco", "PHObject");
  runNode->addNode(paramDecoNode);

  ////////////////////////////////////////////////////////////////  
  PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (!eventNode) {
    eventNode = new PHCompositeNode("DST");
    topNode->addNode(eventNode);
  }
  
  PHIODataNode<PHObject>* eventHeaderNode = new PHIODataNode<PHObject>(new SQEvent_v2(),"SQEvent", "PHObject");
  eventNode->addNode(eventHeaderNode);

  PHIODataNode<PHObject>* hardEventNode = new PHIODataNode<PHObject>(new SQHardEvent_v1(),"SQHardEvent", "PHObject");
  eventNode->addNode(hardEventNode);

  PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(new SQHitVector_v1(), "SQHitVector", "PHObject");
  eventNode->addNode(hitNode);

  PHIODataNode<PHObject>* triggerhitNode = new PHIODataNode<PHObject>(new SQHitVector_v1(), "SQTriggerHitVector", "PHObject");
  eventNode->addNode(triggerhitNode);
  
  //PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  //if (!PrdfNode)
  //  {
  //    PHDataNode<Event> *newNode = new PHDataNode<Event>(evt,"EVIO","Event");
  //    topNode->addNode(newNode);
  //  }
  syncobject = new SyncObjectv2();

  for (auto it = LIST_TIMERS.begin(); it != LIST_TIMERS.end(); it++) {
    string name = *it;
    m_timers[name] = new PHTimer2(name);
  }

  return ;
}

Fun4AllEVIOInputManager::~Fun4AllEVIOInputManager()
{
  if (isopen)
    {
      fileclose();
    }
  if (parser) delete parser;
  delete syncobject;
  for (auto it = m_timers.begin(); it != m_timers.end(); it++) delete it->second;
}

int Fun4AllEVIOInputManager::fileopen(const string &filenam)
{
  if (isopen)
    {
      cout << "Closing currently open file "
           << filename
           << " and opening " << filenam << endl;
      fileclose();
    }
  filename = filenam;
  //FROG frog;
  //string fname = frog.location(filename.c_str());
  string fname = filename;
  if (verbosity > 0)
    {
      cout << ThisName << ": opening file " << filename.c_str() << endl;
    }

  events_thisfile = 0;
  parser->dec_par.verb = Verbosity();
  int status = parser->OpenCodaFile(fname);
  if (status!=0) {
    cout << PHWHERE << ThisName << ": could not open file " << fname << " with status = " << status << "." << endl;
    return -1;
  }
  //pair<int, int> runseg = Fun4AllUtils::GetRunSegment(fname);
  //segment = runseg.second;
  segment = 0;
  isopen = 1;
  AddToFileOpened(fname); // add file to the list of files which were opened
  return 0;
}

int Fun4AllEVIOInputManager::run(const int nevents)
{
  //cout << "Fun4AllEVIOInputManager::run(): " << nevents << endl;
  readagain:

  m_timers["run_open"]->restart();
  if (!isopen)
    {
      if (filelist.empty())

	{
	  if (verbosity > 0)
	    {
	      cout << Name() << ": No Input file open" << endl;
	    }
	  return -1;
        }
      else
        {
          if (OpenNextFile())
            {
              cout << Name() << ": No Input file from filelist opened" << endl;
              return -1;
            }
        }
    }
  m_timers["run_open"]->stop();

  m_timers["run_get_node"]->restart();
  if (verbosity > 3) cout << "Getting Event from " << Name() << endl;
  //  cout << "running event " << nevents << endl;
  //PHNodeIterator iter(topNode);
  SQRun* run_header = findNode::getClass<SQRun>(topNode, "SQRun");
  if (!run_header) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  SQSpillMap* spill_map = findNode::getClass<SQSpillMap>(topNode, "SQSpillMap");
  if (!spill_map) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  SQIntMap* hard_spill_map = findNode::getClass<SQIntMap>(topNode, "SQHardSpillMap");
  if (!hard_spill_map) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  SQEvent* event_header = findNode::getClass<SQEvent>(topNode, "SQEvent");
  if (!event_header) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  SQHardEvent* hard_evt = findNode::getClass<SQHardEvent>(topNode, "SQHardEvent");
  if (!hard_evt) return Fun4AllReturnCodes::ABORTEVENT;

  SQHitVector* hit_vec = findNode::getClass<SQHitVector>(topNode, "SQHitVector");
  if (!hit_vec) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  SQHitVector* trig_hit_vec = findNode::getClass<SQHitVector>(topNode, "SQTriggerHitVector");
  if (!trig_hit_vec) {
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  m_timers["run_get_node"]->stop();

  m_timers["run_get_event"]->restart();
  //PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  EventData* ed = 0;
  SpillData* sd = 0;
  RunData  * rd = 0;
//  if (save_evt) // if an event was pushed back, copy saved pointer and reset save_evt pointer
//    {
//      cerr << "!!ERROR!!  save_evt is not supported yet." << endl;
//      evt = save_evt;
//      save_evt = NULL;
//      events_thisfile--;
//      events_total--;
//    }
//  else
    {
      //int * data_ptr = NULL;
      //unsigned int coda_id = 0;
      //if(parser->coda->NextCodaEvent(coda_id, data_ptr))
      //  evt = new EVIO_Event(data_ptr);
      parser->NextPhysicsEvent(ed, sd, rd);
    }
  m_timers["run_get_event"]->stop();

  if (!ed)
    {
      fileclose();
      goto readagain;
    }
  if (verbosity > 3) cout << ThisName << ":  run " << rd->run_id << ", spill " << sd->spill_id << ", event " << ed->event.eventID << endl;

  if (events_total == 0) {
    m_timers["run_set_param"]->restart();
    SQParamDeco* sqpd = findNode::getClass<SQParamDeco>(topNode, "SQParamDeco");
    if (!sqpd) return Fun4AllReturnCodes::ABORTEVENT;
    DecoParam* dp = &parser->dec_par;
    sqpd->set_variable(dp->chan_map_taiwan.GetParamID(), dp->chan_map_taiwan.GetMapID());
    sqpd->set_variable(dp->chan_map_v1495 .GetParamID(), dp->chan_map_v1495 .GetMapID());
    sqpd->set_variable(dp->chan_map_scaler.GetParamID(), dp->chan_map_scaler.GetMapID());
    m_timers["run_set_param"]->stop();
  }

  events_total++;
  events_thisfile++;

  m_timers["run_set_run_data"]->restart();

  SetRunNumber                (rd->run_id);
  mySyncManager->PrdfEvents   (events_thisfile);
  mySyncManager->SegmentNumber(sd->spill_id);
  mySyncManager->CurrentEvent (ed->event.eventID);
  syncobject->EventCounter    (events_thisfile);
  syncobject->SegmentNumber   (sd->spill_id);
  syncobject->RunNumber       (rd->run_id);
  syncobject->EventNumber     (ed->event.eventID);

  run_header->set_run_id         (rd->run_id);
  run_header->set_unix_time_begin(rd->utime_b);
  run_header->set_unix_time_end  (rd->utime_e);
  for (int ii = 0; ii < 5; ii++) {
    run_header->set_fpga_enabled (ii, rd->fpga_enabled [ii]);
    run_header->set_fpga_prescale(ii, rd->fpga_prescale[ii]);
    run_header->set_nim_enabled  (ii, rd-> nim_enabled [ii]);
  }
  for (int ii = 0; ii < 3; ii++) {
    run_header->set_nim_prescale(ii, rd->nim_prescale[ii]);
  }
  run_header->set_run_desc       (rd->run_desc);
  run_header->set_n_fee_event    (rd->n_fee_event);
  run_header->set_n_fee_prescale (rd->n_fee_prescale);
  run_header->set_n_run_desc     (rd->n_run_desc);
  run_header->set_n_spill        (rd->n_spill);
  run_header->set_n_evt_all      (rd->n_evt_all);
  run_header->set_n_evt_dec      (rd->n_evt_dec);
  run_header->set_n_phys_evt     (rd->n_phys_evt);
  run_header->set_n_phys_evt_bad (rd->n_phys_evt_bad);
  run_header->set_n_flush_evt    (rd->n_flush_evt);
  run_header->set_n_flush_evt_bad(rd->n_flush_evt_bad);
  run_header->set_n_hit          (rd->n_hit);
  run_header->set_n_t_hit        (rd->n_t_hit);
  run_header->set_n_hit_bad      (rd->n_hit_bad);
  run_header->set_n_t_hit_bad    (rd->n_t_hit_bad);
  run_header->set_n_v1495        (rd->n_v1495);
  run_header->set_n_v1495_d1ad   (rd->n_v1495_d1ad);
  run_header->set_n_v1495_d2ad   (rd->n_v1495_d2ad);
  run_header->set_n_v1495_d3ad   (rd->n_v1495_d3ad);
  m_timers["run_set_run_data"]->stop();

  m_timers["run_set_spill_data"]->restart();
  SQSpill* spill = spill_map->get(sd->spill_id);
  if (! spill) {
    spill = new SQSpill_v2();
    spill->set_spill_id    (sd->spill_id);
    spill->set_run_id      (sd->run_id  );
    spill->set_target_pos  (sd->targ_pos);
    for (ScalerDataList::iterator it = sd->list_scaler.begin(); it != sd->list_scaler.end(); it++) {
      SQScaler_v1 obj;
      obj.set_name (it->name );
      obj.set_count(it->value);
      if (it->type == MainDaqParser::TYPE_BOS) {
        obj.set_type(SQScaler::BOS);
        spill->get_bos_scaler_list()->insert(it->name, &obj);
      } else {
        obj.set_type(SQScaler::EOS);
        spill->get_eos_scaler_list()->insert(it->name, &obj);
      }
    }
    for (SlowControlDataList::iterator it = sd->list_slow_cont.begin(); it != sd->list_slow_cont.end(); it++) {
      SQSlowCont_v1 obj;
      obj.set_time_stamp(it->ts   );
      obj.set_name      (it->name );
      obj.set_value     (it->value);
      obj.set_type      (it->type );
      spill->get_slow_cont_list()->insert(it->name, &obj);
    }
    spill_map->insert(spill);
  }
  SQHardSpill* hard_spill = (SQHardSpill*)hard_spill_map->get(sd->spill_id);
  if (! hard_spill) {
    SQHardSpill_v1 obj;
    obj.set_bos_coda_id (sd->bos_coda_id );
    obj.set_bos_vme_time(sd->bos_vme_time);
    obj.set_eos_coda_id (sd->eos_coda_id );
    obj.set_eos_vme_time(sd->eos_vme_time);
    obj.set_time_input  (sd->time_input   );
    obj.set_time_decode (sd->time_decode  );
    obj.set_time_map    (sd->time_map     );
    hard_spill = (SQHardSpill*)hard_spill_map->insert(sd->spill_id, &obj);

    Fun4AllServer::instance()->ResetSpillTimer();
  }
  double time_subsys, time_output;
  Fun4AllServer::instance()->ReadSpillTimer(time_subsys, time_output);
  hard_spill->set_time_subsys(time_subsys);
  hard_spill->set_time_output(time_output);

  m_timers["run_set_spill_data"]->stop();

  m_timers["run_set_event_data"]->restart();
  event_header->set_run_id       (ed->event.runID  );
  event_header->set_spill_id     (ed->event.spillID);
  event_header->set_event_id     (ed->event.eventID);
  event_header->set_data_quality (ed->event.dataQuality);
  event_header->set_trigger      (SQEvent::MATRIX1, ed->event.MATRIX[0]);
  event_header->set_trigger      (SQEvent::MATRIX2, ed->event.MATRIX[1]);
  event_header->set_trigger      (SQEvent::MATRIX3, ed->event.MATRIX[2]);
  event_header->set_trigger      (SQEvent::MATRIX4, ed->event.MATRIX[3]);
  event_header->set_trigger      (SQEvent::MATRIX5, ed->event.MATRIX[4]);
  event_header->set_trigger      (SQEvent::NIM1   , ed->event.NIM   [0]);
  event_header->set_trigger      (SQEvent::NIM2   , ed->event.NIM   [1]);
  event_header->set_trigger      (SQEvent::NIM3   , ed->event.NIM   [2]);
  event_header->set_trigger      (SQEvent::NIM4   , ed->event.NIM   [3]);
  event_header->set_trigger      (SQEvent::NIM5   , ed->event.NIM   [4]);
  for (int ii=0; ii<4; ii++) event_header->set_qie_presum(ii, ed->event.sums[ii]);
  event_header->set_qie_trigger_count(ed->event.triggerCount);
  event_header->set_qie_turn_id      (ed->event.turnOnset);
  event_header->set_qie_rf_id        (ed->event.rfOnset);
  for (int ii=0; ii<33; ii++) event_header->set_qie_rf_intensity(ii-16, ed->event.rf[ii]);

  hard_evt->set_coda_event_id(ed->event.codaEventID);
  hard_evt->set_vme_time     (ed->event.vmeTime);
  for (int ii=0; ii<5; ii++) {
    hard_evt->set_raw_matrix      (ii, ed->event.RawMATRIX     [ii]);
    hard_evt->set_after_inh_matrix(ii, ed->event.AfterInhMATRIX[ii]);
  }
  hard_evt->set_flag_v1495        (ed->event.flag_v1495);
  hard_evt->set_n_board_qie       (ed->n_qie   );
  hard_evt->set_n_board_v1495     (ed->n_v1495 );
  hard_evt->set_n_board_taiwan    (ed->n_tdc   );
  hard_evt->set_n_board_trig_bit  (ed->n_trig_b);
  hard_evt->set_n_board_trig_count(ed->n_trig_c);
  m_timers["run_set_event_data"]->stop();

  m_timers["run_set_hit_data"]->restart();
  for (HitDataList::iterator it = ed->list_hit.begin(); it != ed->list_hit.end(); it++) {
    HitData* hd = &*it;
    SQHit* hit = new SQHit_v1();
    hit->set_hit_id     (hd->id  );
    hit->set_detector_id(hd->det );
    hit->set_element_id (hd->ele );
    hit->set_level      (hd->lvl );
    hit->set_tdc_time   (hd->time);
    hit->set_drift_distance(0);
    hit->set_pos           (0);
    hit_vec->push_back(hit);
    delete hit;
  }

  for (HitDataList::iterator it = ed->list_hit_trig.begin(); it != ed->list_hit_trig.end(); it++) {
    HitData* hd = &*it;
    SQHit* hit = new SQHit_v1();
    hit->set_hit_id     (hd->id  );
    hit->set_detector_id(hd->det );
    hit->set_element_id (hd->ele );
    hit->set_level      (hd->lvl );
    hit->set_tdc_time   (hd->time);
    hit->set_drift_distance(0);
    hit->set_pos           (0);
    trig_hit_vec->push_back(hit);
    delete hit;
  }
  m_timers["run_set_hit_data"]->stop();

  // check if the local SubsysReco discards this event
  if (RejectEvent() != Fun4AllReturnCodes::EVENT_OK)
    {
      ResetEvent();
      goto readagain;
    }
  return 0;
}

int Fun4AllEVIOInputManager::fileclose()
{
  if (!isopen)
    {
      cout << Name() << ": fileclose: No Input file open" << endl;
      return -1;
    }

  SQRun* sqrun = findNode::getClass<SQRun>(topNode, "SQRun");
  if (sqrun) {
    RunData* rd = parser->GetRunData();
    sqrun->set_unix_time_end(rd->utime_e);
  }

  parser->End();
  cout << "Fun4AllEVIOInputManager: Timer:\n";
  for (auto it = LIST_TIMERS.begin(); it != LIST_TIMERS.end(); it++) {
    m_timers[*it]->print_stat();
  }

  isopen = 0;
  // if we have a file list, move next entry to top of the list
  // or repeat the same entry again
  if (!filelist.empty())
    {
      if (repeat)
        {
          filelist.push_back(*(filelist.begin()));
          if (repeat > 0)
            {
              repeat--;
            }
        }
      filelist.pop_front();
    }

  return 0;
}


void
Fun4AllEVIOInputManager::Print(const string &what) const
{
  Fun4AllInputManager::Print(what);
  return ;
}

int
Fun4AllEVIOInputManager::OpenNextFile()
{
  while (!filelist.empty())
    {
      list<string>::const_iterator iter = filelist.begin();
      if (verbosity)
        {
          cout << PHWHERE << " opening next file: " << *iter << endl;
        }
      if (fileopen(*iter))
        {
          cout << PHWHERE << " could not open file: " << *iter << endl;
          filelist.pop_front();
        }
      else
        {
          return 0;
        }

    }
  return -1;
}

int
Fun4AllEVIOInputManager::ResetEvent()
{
  //PHNodeIterator iter(topNode);
  //PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  //PrdfNode->setData(NULL); // set pointer in Node to NULL before deleting it
  //delete evt;
  //evt = NULL;
  syncobject->Reset();
  return 0;
}

int
Fun4AllEVIOInputManager::PushBackEvents(const int i)
{
  cerr << "!!ERROR!!  PushBackEvents():  Not implemented yet." << endl;
  // PushBackEvents is supposedly pushing events back on the stack which works
  // easily with root trees (just grab a different entry) but hard in these HepMC ASCII files.
  // A special case is when the synchronization fails and we need to only push back a single
  // event. In this case we save the evt pointer as save_evt which is used in the run method
  // instead of getting the next event.
//  if (i > 0)
//    {
//      if (i == 1 && evt) // check on evt pointer makes sure it is not done from the cmd line
//	{
//	  save_evt = evt;
//	  return 0;
//	}
//      cout << PHWHERE << ThisName
//           << " Fun4AllEVIOInputManager cannot push back " << i << " events into file"
//           << endl;
//      return -1;
//    }
//  if (!parser->coda)
//    {
//      cout << PHWHERE << ThisName
//	   << " no file open" << endl;
//      return -1;
//    }
  // Skipping events is implemented as
  // pushing a negative number of events on the stack, so in order to implement
  // the skipping of events we read -i events.
//  int nevents = -i; // negative number of events to push back -> skip num events
//  int errorflag = 0;
//  while (nevents > 0 && ! errorflag)
//    {
//			int * data_ptr = NULL;
//			unsigned int coda_id = 0;
//			if(parser->coda->NextCodaEvent(coda_id, data_ptr))
//				evt = new EVIO_Event(data_ptr);
//      if (! evt)
//	{
//	  cout << "Error after skipping " << i - nevents 
//	       << " file exhausted?" << endl;
//	  errorflag = -1;
//          fileclose();
//	}
//      else
//	{
//	  if (verbosity > 3)
//	    {
//	  		//TODO implement this
//	      //cout << "Skipping evt no: " << evt->getEvtSequence() << endl;
//	    }
//	}
//      delete evt;
//      nevents--;
//    }
//  return errorflag;
  return -1;
}

int
Fun4AllEVIOInputManager::GetSyncObject(SyncObject **mastersync)
{
  // here we copy the sync object from the current file to the
  // location pointed to by mastersync. If mastersync is a 0 pointer
  // the syncobject is cloned. If mastersync allready exists the content
  // of syncobject is copied
  if (!(*mastersync))
    {
      if (syncobject) *mastersync = syncobject->clone();
    }
  else
    {
      *(*mastersync) = *syncobject; // copy syncobject content
    }
  return Fun4AllReturnCodes::SYNC_OK;
}

int
Fun4AllEVIOInputManager::SyncIt(const SyncObject *mastersync)
{
  if (!mastersync)
    {
      cout << PHWHERE << Name() << " No MasterSync object, cannot perform synchronization" << endl;
      cout << "Most likely your first file does not contain a SyncObject and the file" << endl;
      cout << "opened by the Fun4AllDstInputManager with Name " << Name() << " has one" << endl;
      cout << "Change your macro and use the file opened by this input manager as first input" << endl;
      cout << "and you will be okay. Fun4All will not process the current configuration" << endl << endl;
      return Fun4AllReturnCodes::SYNC_FAIL;
    }
  int iret = syncobject->Different(mastersync);
  if (iret)
    {
      cout << "big problem" << endl;
      exit(1);
    }
  return Fun4AllReturnCodes::SYNC_OK;
}

void Fun4AllEVIOInputManager::SetOnline(const bool is_online)
{
  parser->dec_par.is_online = is_online;
  parser->GetCoda()->SetOnline(is_online);
}

void Fun4AllEVIOInputManager::EventSamplingFactor(const int factor)
{
  parser->dec_par.sampling = factor;
}

void Fun4AllEVIOInputManager::DirParam(const std::string dir)
{
  parser->dec_par.dir_param = dir;
}

void Fun4AllEVIOInputManager::PretendSpillInterval(const int sec)
{
  parser->dec_par.time_wait = sec;
}

void Fun4AllEVIOInputManager::UseLocalSpillID(const bool use)
{
  parser->UseLocalSpillID(use);
}

bool Fun4AllEVIOInputManager::UseLocalSpillID() const
{
  return parser->UseLocalSpillID();
}
