#include <event/EVIO_Event.h>
#include <evio/ManageCodaInput.h>

#include "Fun4AllEVIOInputManager.h"
#include "Fun4AllServer.h"
#include "Fun4AllSyncManager.h"
#include "Fun4AllReturnCodes.h"
#include "Fun4AllUtils.h"

#include <ffaobjects/RunHeader.h>
#include <ffaobjects/SyncObjectv2.h>
//#include <frog/FROG.h>

#include <phool/getClass.h>
#include <phool/PHCompositeNode.h>
#include <phool/PHDataNode.h>
#include <phool/recoConsts.h>
#include <cstdlib>
#include <memory>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

Fun4AllEVIOInputManager::Fun4AllEVIOInputManager(const string &name, const string &topnodename) :
 Fun4AllInputManager(name, ""),
 segment(-999),
 isopen(0),
 events_total(0),
 events_thisfile(0),
 topNodeName(topnodename),
 evt(NULL),
 save_evt(NULL),
 coda(NULL)
{
  Fun4AllServer *se = Fun4AllServer::instance();
  topNode = se->topNode(topNodeName.c_str());
  PHNodeIterator iter(topNode);
  PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  if (!PrdfNode)
    {
      PHDataNode<Event> *newNode = new PHDataNode<Event>(evt,"EVIO","Event");
      topNode->addNode(newNode);
    }
  syncobject = new SyncObjectv2();
  return ;
}

Fun4AllEVIOInputManager::~Fun4AllEVIOInputManager()
{
  if (isopen)
    {
      fileclose();
    }
  delete syncobject;
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

  coda = new ManageCodaInput();
  events_thisfile = 0;

  const int file_size_min = 32768;
  const int sec_wait      = 15;
  const int n_wait        = 40;
  int status = coda->OpenFile(fname, file_size_min, sec_wait, n_wait);

  if (status!=0) {
  	cerr << "!!ERROR!! Failed at file open (" << status << ").  Exit.\n";
      delete coda;
      coda = NULL;
      cout << PHWHERE << ThisName << ": could not open file " << fname << endl;
      return -1;
  }
  pair<int, int> runseg = Fun4AllUtils::GetRunSegment(fname);
  segment = runseg.second;
  isopen = 1;
  AddToFileOpened(fname); // add file to the list of files which were opened
  return 0;
}

int Fun4AllEVIOInputManager::run(const int nevents)
{
  readagain:
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
  if (verbosity > 3)
    {
      cout << "Getting Event from " << Name() << endl;
    }
  //  cout << "running event " << nevents << endl;
  PHNodeIterator iter(topNode);
  PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  if (save_evt) // if an event was pushed back, copy saved pointer and reset save_evt pointer
    {
      evt = save_evt;
      save_evt = NULL;
      events_thisfile--;
      events_total--;
    }
  else
    {
      int * data_ptr = NULL;
      unsigned int coda_id = 0;
			if(coda->NextEvent(coda_id, data_ptr))
				evt = new EVIO_Event(data_ptr);
    }
  PrdfNode->setData(evt);
  if (!evt)
    {
      fileclose();
      goto readagain;
    }
  if (verbosity > 1)
    {
  		//TODO implement this
      //cout << ThisName << " EVIO run " << evt->getRunNumber() << ", evt no: " << evt->getEvtSequence() << endl;
    }
  events_total++;
  events_thisfile++;

  //TODO implement these
//  SetRunNumber(evt->getRunNumber());
//  mySyncManager->PrdfEvents(events_thisfile);
//  mySyncManager->SegmentNumber(segment);
//  mySyncManager->CurrentEvent(evt->getEvtSequence());
//  syncobject->EventCounter(events_thisfile);
//  syncobject->SegmentNumber(segment);
//  syncobject->RunNumber(evt->getRunNumber());
//  syncobject->EventNumber(evt->getEvtSequence());


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
  delete coda;
  coda = NULL;
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
  PHNodeIterator iter(topNode);
  PHDataNode<Event> *PrdfNode = dynamic_cast<PHDataNode<Event> *>(iter.findFirst("PHDataNode","EVIO"));
  PrdfNode->setData(NULL); // set pointer in Node to NULL before deleting it
  delete evt;
  evt = NULL;
  syncobject->Reset();
  return 0;
}

int
Fun4AllEVIOInputManager::PushBackEvents(const int i)
{
  // PushBackEvents is supposedly pushing events back on the stack which works
  // easily with root trees (just grab a different entry) but hard in these HepMC ASCII files.
  // A special case is when the synchronization fails and we need to only push back a single
  // event. In this case we save the evt pointer as save_evt which is used in the run method
  // instead of getting the next event.
  if (i > 0)
    {
      if (i == 1 && evt) // check on evt pointer makes sure it is not done from the cmd line
	{
	  save_evt = evt;
	  return 0;
	}
      cout << PHWHERE << ThisName
           << " Fun4AllEVIOInputManager cannot push back " << i << " events into file"
           << endl;
      return -1;
    }
  if (!coda)
    {
      cout << PHWHERE << ThisName
	   << " no file open" << endl;
      return -1;
    }
  // Skipping events is implemented as
  // pushing a negative number of events on the stack, so in order to implement
  // the skipping of events we read -i events.
  int nevents = -i; // negative number of events to push back -> skip num events
  int errorflag = 0;
  while (nevents > 0 && ! errorflag)
    {
			int * data_ptr = NULL;
			unsigned int coda_id = 0;
			if(coda->NextEvent(coda_id, data_ptr))
				evt = new EVIO_Event(data_ptr);
      if (! evt)
	{
	  cout << "Error after skipping " << i - nevents 
	       << " file exhausted?" << endl;
	  errorflag = -1;
          fileclose();
	}
      else
	{
	  if (verbosity > 3)
	    {
	  		//TODO implement this
	      //cout << "Skipping evt no: " << evt->getEvtSequence() << endl;
	    }
	}
      delete evt;
      nevents--;
    }
  return errorflag;
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
