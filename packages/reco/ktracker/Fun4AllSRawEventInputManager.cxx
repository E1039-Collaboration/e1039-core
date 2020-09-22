#include "Fun4AllSRawEventInputManager.h"

#include "SRawEvent.h"

#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v2.h>
#include <interface_main/SQSpillMap_v1.h>
#include <interface_main/SQStringMap.h>
#include <interface_main/SQScaler_v1.h>
#include <interface_main/SQSlowCont_v1.h>
 
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllSyncManager.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/PHTFileServer.h>

#include <ffaobjects/RunHeader.h>
#include <ffaobjects/SyncObjectv2.h>

#include <phool/getClass.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHDataNode.h>
#include <phool/recoConsts.h>
#include <cstdlib>
#include <memory>

#include <TFile.h>
#include <TTree.h>

//#include <boost/tokenizer.hpp>
//#include <boost/foreach.hpp>
//#include <boost/lexical_cast.hpp>

using namespace std;

Fun4AllSRawEventInputManager::Fun4AllSRawEventInputManager(const string& name, const string& topnodename):
  Fun4AllInputManager(name, ""),
  segment(-999),
  isopen(0),
  events_total(0),
  events_thisfile(0),
  topNodeName(topnodename),
  _tree_name("save"),
  _branch_name("rawEvent"),
  _enable_e1039_translation(false),
  run_header(nullptr),
  spill_map(nullptr),
  event_header(nullptr),
  hit_vec(nullptr),
  trig_hit_vec(nullptr)
{
  Fun4AllServer* se = Fun4AllServer::instance();
  topNode = se->topNode(topNodeName.c_str());
  PHNodeIterator iter(topNode);

  PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!runNode) {
    runNode = new PHCompositeNode("RUN");
    topNode->addNode(runNode);
  }

  PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));
  if (!eventNode) {
    eventNode = new PHCompositeNode("DST");
    topNode->addNode(eventNode);
  }

  _srawEvent = new SRawEvent();
  PHIODataNode<PHObject>* rawEventNode = new PHIODataNode<PHObject>(_srawEvent, "SRawEvent", "PHObject");
  eventNode->addNode(rawEventNode);

  syncobject = new SyncObjectv2();
}

Fun4AllSRawEventInputManager::~Fun4AllSRawEventInputManager()
{
  if(isopen)
  {
    fileclose();
  }
  delete syncobject;
}

void Fun4AllSRawEventInputManager::enable_E1039_translation()
{
  if(!_enable_e1039_translation)
  {
    _enable_e1039_translation = true;

    Fun4AllServer* se = Fun4AllServer::instance();
    topNode = se->topNode(topNodeName.c_str());

    PHNodeIterator iter(topNode);
    PHCompositeNode* runNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "RUN"));
    PHCompositeNode* eventNode = static_cast<PHCompositeNode*>(iter.findFirst("PHCompositeNode", "DST"));

    run_header = new SQRun_v1();
    PHIODataNode<PHObject>* runHeaderNode = new PHIODataNode<PHObject>(run_header, "SQRun", "PHObject");
    runNode->addNode(runHeaderNode);
  
    spill_map = new SQSpillMap_v1();
    PHIODataNode<PHObject>* spillNode = new PHIODataNode<PHObject>(spill_map, "SQSpillMap", "PHObject");
    runNode->addNode(spillNode);

    event_header = new SQEvent_v1();
    PHIODataNode<PHObject>* eventHeaderNode = new PHIODataNode<PHObject>(event_header, "SQEvent", "PHObject");
    eventNode->addNode(eventHeaderNode);

    hit_vec = new SQHitVector_v1();
    PHIODataNode<PHObject>* hitNode = new PHIODataNode<PHObject>(hit_vec, "SQHitVector", "PHObject");
    eventNode->addNode(hitNode);

    trig_hit_vec = new SQHitVector_v1();
    PHIODataNode<PHObject>* triggerhitNode = new PHIODataNode<PHObject>(trig_hit_vec, "SQTriggerHitVector", "PHObject");
    eventNode->addNode(triggerhitNode);
  }
}

void Fun4AllSRawEventInputManager::E906ToE1039()
{
  run_header->set_run_id(_srawEvent->getRunID());

  SQSpill* spill = spill_map->get(_srawEvent->getSpillID());
  if(!spill) {
    spill = new SQSpill_v2();
    spill->set_spill_id(_srawEvent->getSpillID());
    spill->set_run_id(_srawEvent->getRunID());

    spill_map->insert(spill);
    run_header->set_n_spill(spill_map->size());
  }

  event_header->set_run_id(_srawEvent->getRunID());
  event_header->set_spill_id(_srawEvent->getSpillID());
  event_header->set_event_id(_srawEvent->getEventID());
  event_header->set_data_quality(0);
  event_header->set_trigger(SQEvent::MATRIX1, _srawEvent->isTriggeredBy(SRawEvent::MATRIX1));
  event_header->set_trigger(SQEvent::MATRIX2, _srawEvent->isTriggeredBy(SRawEvent::MATRIX2));
  event_header->set_trigger(SQEvent::MATRIX3, _srawEvent->isTriggeredBy(SRawEvent::MATRIX3));
  event_header->set_trigger(SQEvent::MATRIX4, _srawEvent->isTriggeredBy(SRawEvent::MATRIX4));
  event_header->set_trigger(SQEvent::MATRIX5, _srawEvent->isTriggeredBy(SRawEvent::MATRIX5));
  event_header->set_trigger(SQEvent::NIM1, _srawEvent->isTriggeredBy(SRawEvent::NIM1));
  event_header->set_trigger(SQEvent::NIM2, _srawEvent->isTriggeredBy(SRawEvent::NIM2));
  event_header->set_trigger(SQEvent::NIM3, _srawEvent->isTriggeredBy(SRawEvent::NIM3));
  event_header->set_trigger(SQEvent::NIM4, _srawEvent->isTriggeredBy(SRawEvent::NIM4));
  event_header->set_trigger(SQEvent::NIM5, _srawEvent->isTriggeredBy(SRawEvent::NIM5));
  event_header->set_qie_turn_id(_srawEvent->getTurnID());
  event_header->set_qie_rf_id(_srawEvent->getRFID());
  for(int i = -16; i < 16; ++i) event_header->set_qie_rf_intensity(i, _srawEvent->getIntensity(i));

  vector<Hit>& hits = _srawEvent->getAllHits();
  for(auto it = hits.begin(); it != hits.end(); ++it) {
    SQHit* hit = new SQHit_v1();
    hit->set_hit_id(it->index);
    hit->set_detector_id(it->detectorID);
    hit->set_element_id(it->elementID);
    hit->set_tdc_time(it->tdcTime);
    hit->set_in_time(it->isInTime());

    hit_vec->push_back(hit);
    delete hit;
  }

  vector<Hit>& trhits = _srawEvent->getTriggerHits();
  for(auto it = hits.begin(); it != hits.end(); ++it) {
    SQHit* hit = new SQHit_v1();
    hit->set_hit_id(it->index);
    hit->set_detector_id(it->detectorID);
    hit->set_element_id(it->elementID);
    hit->set_tdc_time(it->tdcTime);
    hit->set_in_time(it->isInTime());
    
    trig_hit_vec->push_back(hit);
    delete hit;
  }
}

int Fun4AllSRawEventInputManager::fileopen(const string &filenam)
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

  //PHTFileServer::get().open(filenam.c_str(), "READ");
  _fin = TFile::Open(filenam.c_str(), "READ");
  _tin = (TTree*) _fin->Get(_tree_name.c_str());

  if (!_tin) {
    cerr << "!!ERROR!! Failed at file open (" << filenam.c_str() << ").  Exit.\n";
    cout << PHWHERE << ThisName << ": could not open file " << fname << endl;
    return -1;
  }

  _tin->SetBranchAddress(_branch_name.c_str(), &_srawEvent);

  //TODO check branch status

  //pair<int, int> runseg = Fun4AllUtils::GetRunSegment(fname);
  //segment = runseg.second;
  segment = 0;
  isopen = 1;
  AddToFileOpened(fname); // add file to the list of files which were opened
  return 0;
}

int Fun4AllSRawEventInputManager::run(const int nevents)
{
  //cout << "Fun4AllSRawEventInputManager::run(): " << nevents << endl;
  readagain:
  if (!isopen) {
		if (filelist.empty()) {
			if (verbosity > 0) {
				cout << Name() << ": No Input file open" << endl;
			}
			return -1;
		} else {
			if (OpenNextFile()) {
				cout << Name() << ": No Input file from filelist opened" << endl;
				return -1;
			}
		}
	}
	if (verbosity > 3) {
		cout << "Getting Event from " << Name() << endl;
	}

	if (events_thisfile>=_tin->GetEntries()) {
		fileclose();
		goto readagain;
	}
  
	_tin->GetEntry(events_thisfile);
	events_thisfile++;
	events_total++;

	if (verbosity > Fun4AllBase::VERBOSITY_A_LOT) {
		_srawEvent->identify();
	}

  SetRunNumber                (_srawEvent->getRunID());
  mySyncManager->PrdfEvents   (events_thisfile);
  mySyncManager->SegmentNumber(_srawEvent->getSpillID());
  mySyncManager->CurrentEvent (_srawEvent->getEventID());

  syncobject->RunNumber       (_srawEvent->getRunID());
  syncobject->EventCounter    (events_thisfile);
  syncobject->SegmentNumber   (_srawEvent->getSpillID());
  syncobject->EventNumber     (_srawEvent->getEventID());

  //translate to E1039 data format if enabled
  if (_enable_e1039_translation) {
    E906ToE1039();
  }

  // check if the local SubsysReco discards this event
  if (RejectEvent() != Fun4AllReturnCodes::EVENT_OK) {
    ResetEvent();
    goto readagain;
  }

  return 0;
}

int Fun4AllSRawEventInputManager::fileclose()
{
	if (!isopen) {
		cout << Name() << ": fileclose: No Input file open" << endl;
		return -1;
	}

	_fin->Close();
	isopen = 0;

	// if we have a file list, move next entry to top of the list
	// or repeat the same entry again
	if (!filelist.empty()) {
		if (repeat) {
			filelist.push_back(*(filelist.begin()));
			if (repeat > 0) {
				repeat--;
			}
		}
		filelist.pop_front();
	}

	return 0;
}


void
Fun4AllSRawEventInputManager::Print(const string &what) const
{
  Fun4AllInputManager::Print(what);
  return ;
}

int
Fun4AllSRawEventInputManager::OpenNextFile()
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
Fun4AllSRawEventInputManager::ResetEvent()
{
  //PHNodeIterator iter(topNode);
  //PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  //PrdfNode->setData(nullptr); // set pointer in Node to nullptr before deleting it
  //delete evt;
  //evt = nullptr;
  syncobject->Reset();
  return 0;
}

int
Fun4AllSRawEventInputManager::PushBackEvents(const int i)
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
//           << " Fun4AllSRawEventInputManager cannot push back " << i << " events into file"
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
//			int * data_ptr = nullptr;
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
Fun4AllSRawEventInputManager::GetSyncObject(SyncObject **mastersync)
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
Fun4AllSRawEventInputManager::SyncIt(const SyncObject *mastersync)
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
