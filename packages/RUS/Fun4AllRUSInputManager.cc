 #include "Fun4AllRUSInputManager.h"
 #include <interface_main/SQHit_v1.h>
 #include <interface_main/SQTrack_v1.h>
 #include <interface_main/SQHitVector_v1.h>
 #include <interface_main/SQTrackVector_v1.h>
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

using namespace std;

//Constructor
Fun4AllRUSInputManager::Fun4AllRUSInputManager(const std::string& name, const std::string& topnodename):
   Fun4AllInputManager(name, ""),
   segment(-999),
   isopen(0),
   events_total(0),
   events_thisfile(0),
   topNodeName(topnodename),
   _tree_name("save"),
   run_header(nullptr),
   spill_map(nullptr),
   event_header(nullptr),
   hit_vec(nullptr),
   _fin(nullptr),
   _tin(nullptr)
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
   syncobject = new SyncObjectv2();
}

Fun4AllRUSInputManager::~Fun4AllRUSInputManager()
 {
   if(isopen)
   {
     fileclose();
   }
   delete syncobject;
 }

void Fun4AllRUSInputManager::VectToE1039() {
	event_header->set_run_id(runID);
	SQSpill* spill = spill_map->get(spillID);
	if(!spill) {
		spill = new SQSpill_v2();
		spill->set_spill_id(spillID);
		spill->set_run_id(runID);
		spill_map->insert(spill);
		run_header->set_n_spill(spill_map->size());
	}
	event_header->set_spill_id(spillID);
	event_header->set_event_id(eventID);
	event_header->set_data_quality(0);
	for(int i = -16; i <= 16; ++i) {event_header->set_qie_rf_intensity(i, rfIntensity[i+16]);} // need to be added
	// Apply the FPGA triggers to the event header
	event_header->set_trigger(SQEvent::MATRIX1, fpgaTrigger[0]);
	event_header->set_trigger(SQEvent::MATRIX2, fpgaTrigger[1]);
	event_header->set_trigger(SQEvent::MATRIX3, fpgaTrigger[2]);
	event_header->set_trigger(SQEvent::MATRIX4, fpgaTrigger[3]);
	event_header->set_trigger(SQEvent::MATRIX5, fpgaTrigger[4]);
	// Apply the NIM triggers to the event header
	event_header->set_trigger(SQEvent::NIM1, nimTrigger[0]);
	event_header->set_trigger(SQEvent::NIM2, nimTrigger[1]);
	event_header->set_trigger(SQEvent::NIM3, nimTrigger[2]);
	event_header->set_trigger(SQEvent::NIM4, nimTrigger[3]);
	event_header->set_trigger(SQEvent::NIM5, nimTrigger[4]);
	event_header->set_qie_turn_id(turnID);
	event_header->set_qie_rf_id(rfID);
	for (size_t i = 0; i < elementID->size(); ++i) {
		SQHit* hit = new SQHit_v1();
		hit->set_hit_id(hitID->at(i));
		hit->set_detector_id(detectorID->at(i));
		hit->set_element_id(elementID->at(i));
		hit->set_tdc_time(tdcTime->at(i));
		hit->set_drift_distance(driftDistance->at(i));
		hit_vec->push_back(hit);
	}
}

int Fun4AllRUSInputManager::fileopen(const std::string &filenam) {
	if (isopen) {
		std::cout << "Closing currently open file "
			<< filename
			<< " and opening " << filenam << std::endl;
		fileclose();
	}   
	filename = filenam;


	if (verbosity > 0) {
		std::cout << ThisName << ": opening file " << filename.c_str() << std::endl;
	}   

	events_thisfile = 0;

	_fin = TFile::Open(filenam.c_str(), "READ"); // Open the file dynamically
	if (!_fin || _fin->IsZombie()) {
		std::cerr << "!!ERROR!! Failed to open file " << filenam << std::endl;
	}

	_tin = (TTree*) _fin->Get(_tree_name.c_str());
	if (!_tin) {
		std::cerr << "!!ERROR!! Tree " << _tree_name << " not found in file " << filenam << std::endl;
		return -1; 
	}
	_tin->SetBranchAddress("eventID", &eventID);    
	_tin->SetBranchAddress("runID", &runID);    
	_tin->SetBranchAddress("spillID", &spillID);    
	_tin->SetBranchAddress("rfID", &rfID);    
	_tin->SetBranchAddress("turnID", &turnID);    
	_tin->SetBranchAddress("fpgaTrigger", fpgaTrigger);
	_tin->SetBranchAddress("nimTrigger", nimTrigger);
	_tin->SetBranchAddress("rfIntensity", rfIntensity);

	_tin->SetBranchAddress("hitID", &hitID);    
	_tin->SetBranchAddress("detectorID", &detectorID);    
	_tin->SetBranchAddress("elementID", &elementID);    
	_tin->SetBranchAddress("driftDistance", &driftDistance);    
	_tin->SetBranchAddress("tdcTime", &tdcTime);    

	segment = 0;
	isopen = 1;
	AddToFileOpened(filenam); // Add file to the list of opened files
	return 0;
}

int Fun4AllRUSInputManager::run(const int nevents) {
    readagain:
    if (!isopen) {
        if (filelist.empty()) {
            if (verbosity > 0) {
                std::cout << Name() << ": No Input file open" << std::endl;
            }
            return -1;
        } else {
            if (OpenNextFile()) {
                std::cout << Name() << ": No Input file from filelist opened" << std::endl;
                return -1;
            }
        }
    }

    if (verbosity > 3) {
        std::cout << "Getting Event from " << Name() << std::endl;
    }

    if (events_thisfile >= _tin->GetEntries()) {
        fileclose();
        goto readagain;
    }

    _tin->GetEntry(events_thisfile);
    events_thisfile++;
    events_total++;

   SetRunNumber                (runID);
   mySyncManager->PrdfEvents   (events_thisfile);
   mySyncManager->SegmentNumber(spillID);
   mySyncManager->CurrentEvent (eventID);
  
   syncobject->RunNumber       (runID);
   syncobject->EventCounter    (events_thisfile);
   syncobject->SegmentNumber   (spillID);
   syncobject->EventNumber     (eventID);
      VectToE1039();
   std::cout << "deugg 2: " <<std::endl;
    if (RejectEvent() != Fun4AllReturnCodes::EVENT_OK) {
        ResetEvent();
        goto readagain;
    }
    return 0;
}

int Fun4AllRUSInputManager::ResetEvent()
{
    syncobject->Reset();
    return 0;
}

int Fun4AllRUSInputManager::fileclose()
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
 Fun4AllRUSInputManager::Print(const string &what) const
 {
   Fun4AllInputManager::Print(what);
   return ;
 }
  
 int
 Fun4AllRUSInputManager::OpenNextFile()
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
 
 int Fun4AllRUSInputManager::PushBackEvents(const int i)
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
 //      {
 //        save_evt = evt;
 //        return 0;
 //      }
 //      cout << PHWHERE << ThisName
 //           << " Fun4AllRUSInputManager cannot push back " << i << " events into file"
 //           << endl;
 //      return -1;
 //    }
 //  if (!parser->coda)
 //    {
 //      cout << PHWHERE << ThisName
 //         << " no file open" << endl;
 //      return -1;
 //    }
   // Skipping events is implemented as
   // pushing a negative number of events on the stack, so in order to implement
   // the skipping of events we read -i events.
 //  int nevents = -i; // negative number of events to push back -> skip num events
 //  int errorflag = 0;
 //  while (nevents > 0 && ! errorflag)
 //    {
 //                      int * data_ptr = nullptr;
 //                      unsigned int coda_id = 0;
 //                      if(parser->coda->NextCodaEvent(coda_id, data_ptr))
 //                              evt = new EVIO_Event(data_ptr);
 //      if (! evt)
 //      {
 //        cout << "Error after skipping " << i - nevents 
 //             << " file exhausted?" << endl;
 //        errorflag = -1;
 //          fileclose();
 //      }
 //      else
 //      {
 //        if (verbosity > 3)
 //          {
 //                      //TODO implement this
 //            //cout << "Skipping evt no: " << evt->getEvtSequence() << endl;
 //          }
 //      }
 //      delete evt;
 //      nevents--;
 //    }
 //  return errorflag;
   return -1;
 }
  
 int
 Fun4AllRUSInputManager::GetSyncObject(SyncObject **mastersync)
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
 Fun4AllRUSInputManager::SyncIt(const SyncObject *mastersync)
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
