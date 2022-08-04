/*
 * KScheduler.cxx
 *
 * Scheduler for KEventJobs in the KJobQueue
 *
 * Author: Noah Wuerfel, nwuerfel@umich.edu
 * ~ AP AP AP AP ~
 * created: 10/14/2020
 *
 */

#include <iostream>
#include <algorithm>
#include <cmath>

#include <phool/recoConsts.h>

#include <TCanvas.h>
#include <TGraphErrors.h>
#include <ktracker/KalmanFastTrackletting.h>
#include "KScheduler.h"

ClassImp(KJob)
ClassImp(KScheduler)

int KJob::verb = 0;

// primary instantiator...
// evPtr is pointer to copy of event that doesn't get clobbered on get entry
KJob::KJob(int jobId, SRawEvent* evPtr, KScheduler* universe, bool copy): jobId(jobId),universe(universe){

    if(Verbose() > 0)
        TThread::Printf("starting KJOB: %i\n", jobId);
    
    jobMutex = new TMutex();
    // make some big memory allocations...
    // check them... somehow....
    // need DEEP copy because event is clobbered by GetEntries
    // src - dest
    if(copy){
        evData = new SRawEvent(*evPtr);
    }
    // 1039 i produce new upstream
    else{
        evData = evPtr;
    }
    recEvData = new SRecEvent();
    assert(evData);
    if(!evData){
        std::cout << "failed to allocate memory for SRawEvent... smaller INPUT_PIPE_DEPTH?\n";
    }
    p_JobStatus=STARTED;
    jobTimer = new TStopwatch();
    isPoison = false;

    // get an available, clean TClonesArr
    universe->ktrkqFSem->Wait();
    universe->ktrkQueueMutex->Lock();
    tracklets = universe->kTrackArrQueue.front();
    universe->kTrackArrQueue.pop();
    universe->ktrkQueueMutex->UnLock();
    universe->ktrkqESem->Post();
    nTracklets = 0;

    return;
};


// poisoned jobs to kill downstream threads
KJob::KJob(bool poisoned){
    assert(poisoned &&"shouldn't call this if not poisoned...");
    isPoison = poisoned;
    return;
}

// failing to initialize everything in poison pill was causing crashouts
KJob::~KJob(){

    if(isPoison)
        return;
    
    delete jobMutex;
    delete evData;
    delete recEvData;
    //p_jobStatus
    if(Verbose() > 0){
        TThread::Printf("Kjob: %i completed in time:\n",jobId);
        jobTimer->Print("u");
    }
    delete jobTimer;
    // clear and yield tracklet array
    // TODO check if "C" needed in arg for tracklets
    tracklets->Clear();
    universe->ktrkqESem->Wait();
    universe->ktrkQueueMutex->Lock();
    universe->kTrackArrQueue.push(tracklets);
    universe->ktrkQueueMutex->UnLock();
    universe->ktrkqFSem->Post();
    return;
};

//sure there's a better way for passing compound args...
// thread_local may not be right?.... root is annoying
struct workerArg{
    KScheduler* kschdPtr;
    unsigned threadId;
};


// FUCK this was tricky need to initialize blecause static
// need static to acess via the member threads because pthreads are typically
// static code
int KScheduler::verb = 0;
bool KScheduler::use_e906_data = true;
TString KScheduler::inputFilename="";
TString KScheduler::outputFilename="";
int KScheduler::completedEvents = 0;

// TODO INCLUDE IBUFFLEN AND NTHREADS IN OPTS
KScheduler::KScheduler(TString inFile, TString outFile) 
  : use_tracklet_reco(false)
{
    // I/O managed by fReader and fReaper
    //this->setInputFilename( p_jobOptsSvc->m_inputFile );
    //this->setOutputFilename( p_jobOptsSvc->m_outputFile );

    //take as args now...
    this->setInputFilename(inFile);
    this->setOutputFilename(outFile);
}

void KScheduler::Init()
{
    int i;
    std::cout << "Initialization of KScheduler ..." << std::endl;
    std::cout << "================================" << std::endl;

    // init services
    std::cout << "starting KScheduler services\n";
    GeomSvc* p_geomSvc = GeomSvc::instance();
    recoConsts* rc = recoConsts::instance();

    //Initialize event reducer
    EventReducer* eventReducer = 0;
    for(i=0;i<NEVENT_REDUCERS;i++){
        eventReducer = new EventReducer(rc->get_CharFlag("EventReduceOpts"));
        assert(eventReducer);
        eventReducerQueue.push(eventReducer);
    }
    #ifdef _ENABLE_KF
        // TODO
    #else
    std::cout << "skipped filter\n";
        // TODO setup fast trackers here when filter gets added
        
    #endif

    //Initialize the kfasttrackers
    KalmanFastTracking* kFastTracker = 0;
    for(i=0;i<NKFAST_TRACKERS;i++){
        // _ENABLE_KF KFT takes PHField* and 
        // TGeoManager* as args
        // only used in fitter
        if (! use_tracklet_reco) kFastTracker = new KalmanFastTracking    (nullptr, nullptr, false);
        else                     kFastTracker = new KalmanFastTrackletting(nullptr, nullptr, false);
        assert(kFastTracker);
        kFastTrkQueue.push(kFastTracker);
    }

    // build TClonesArrays for tracklet outputs
    TClonesArray* trackletArray = 0;
    for(i=0; i<NTHREADS; i++){
        trackletArray= new TClonesArray("Tracklet",1000);
        trackletArray->BypassStreamer();
        assert(trackletArray);
        kTrackArrQueue.push(trackletArray);
    }

    // worker threads
    for(i = 0; i<NTHREADS; i++){
        workThreadArr[i] = 0;
    }

    // net time for alljobs
    avgTimer = new TStopwatch();
    totalTimer = new TStopwatch();

    // lock for wArg structure when initializing structs
    wArgMutex = new TMutex();
    fReaderMutex = new TMutex();

    // pipeline inside
    newJobQueuePutMutex = new TMutex();
    newJobQueueTakeMutex = new TMutex();

    // pipeline evReducer
    evRedQueuePutMutex = new TMutex();
    evRedQueueTakeMutex = new TMutex();

    // pipeline finder
    kFTrkQueuePutMutex = new TMutex();
    kFTrkQueueTakeMutex = new TMutex();

    // pipeline out
    cmpJobQueuePutMutex = new TMutex();
    cmpJobQueueTakeMutex = new TMutex();

    //tracklet arr
    ktrkQueueMutex = new TMutex();
   
//    jobSem = new TSemaphore(MAXJOBS);
    njqFSem = new TSemaphore(0);
    njqESem = new TSemaphore(INPUT_PIPE_DEPTH);

    // evReducer
    erqFSem = new TSemaphore(NEVENT_REDUCERS);
    erqESem = new TSemaphore(0);

    // finder
    kftqFSem = new TSemaphore(NKFAST_TRACKERS);
    kftqESem = new TSemaphore(0);

    // trackletArrays
    ktrkqFSem = new TSemaphore(NTHREADS);
    ktrkqESem = new TSemaphore(0);

    // output-stage pipeline
    cjqFSem = new TSemaphore(0);
    cjqESem = new TSemaphore(OUTPUT_PIPE_DEPTH);

    // inits TThreads to 0
    fRDPtr = 0;
    fRPPtr = 0;


    //Initialize hodoscope IDs
    detectorIDs_mask[0] = p_geomSvc->getDetectorIDs("H1");
    detectorIDs_mask[1] = p_geomSvc->getDetectorIDs("H2");
    detectorIDs_mask[2] = p_geomSvc->getDetectorIDs("H3");
    detectorIDs_mask[3] = p_geomSvc->getDetectorIDs("H4");
    detectorIDs_maskX[0] = p_geomSvc->getDetectorIDs("H1[TB]");
    detectorIDs_maskX[1] = p_geomSvc->getDetectorIDs("H2[TB]");
    detectorIDs_maskX[2] = p_geomSvc->getDetectorIDs("H3[TB]");
    detectorIDs_maskX[3] = p_geomSvc->getDetectorIDs("H4[TB]");
    detectorIDs_maskY[0] = p_geomSvc->getDetectorIDs("H1[LR]");
    detectorIDs_maskY[1] = p_geomSvc->getDetectorIDs("H2[LR]");
    detectorIDs_maskY[2] = p_geomSvc->getDetectorIDs("H4Y1[LR]");
    detectorIDs_maskY[3] = p_geomSvc->getDetectorIDs("H4Y2[LR]");
    detectorIDs_muidHodoAid[0] = p_geomSvc->getDetectorIDs("H4[TB]");
    detectorIDs_muidHodoAid[1] = p_geomSvc->getDetectorIDs("H4Y");



    return;

};

KScheduler::~KScheduler(){

    int i = 0;

    //TODO kill remaning threads and clean memory for them
    /*
    for(int i = 0; i<NTHREADS; i++){
        delete jobMutexArr[i];
    }
    */

    
    delete fRDPtr;
    fRDPtr = 0;
    delete fRPPtr;
    fRPPtr = 0;
    for(i=0;i<NTHREADS;i++){
        assert(workThreadArr[i]);
        delete workThreadArr[i];
        workThreadArr[i]=0;
    }

    delete wArgMutex;
    delete fReaderMutex;

    // first stage of pipeline
    delete newJobQueuePutMutex;
    delete newJobQueueTakeMutex;

    // eventReducer
    delete evRedQueuePutMutex;
    delete evRedQueueTakeMutex;
    // TODO CLEAN UP EVENTREDUCERS
    EventReducer* er = 0;
    for(i=0;i<NEVENT_REDUCERS;i++){
        er = eventReducerQueue.front(); 
        eventReducerQueue.pop();
        delete er;
    }
    //delete eventReducerQueue;
    
    // finder
    delete kFTrkQueuePutMutex;
    delete kFTrkQueueTakeMutex;

    KalmanFastTracking* ft = 0;
    for(i=0; i<NKFAST_TRACKERS; i++){
        ft = kFastTrkQueue.front();
        kFastTrkQueue.pop();
        delete ft;
    }

    TClonesArray* trkArr= 0;
    for(i=0; i<NTHREADS; i++){
        trkArr = kTrackArrQueue.front();
        assert(trkArr);
        kTrackArrQueue.pop();
        delete trkArr;
    }

    // second stage of pipeline
    delete cmpJobQueuePutMutex;
    delete cmpJobQueueTakeMutex;

    //tracklet arr mutex
    delete ktrkQueueMutex;
    
    // first stage of pipeline sems
    delete njqFSem;
    delete njqESem;

    // ev reducer
    delete erqFSem;
    delete erqESem;

    // finder
    delete kftqFSem;
    delete kftqESem;

    // tracklet arrays sems
    delete ktrkqFSem;
    delete ktrkqESem;

    //output stage of pipeline sems
    delete cjqFSem;
    delete cjqESem;

    // timer
    delete avgTimer;
    TThread::Printf("Total Time and average per event below:");
    // something with printing or formatting is wrong here...
    // yeah the formatting is stupid.. I pulled this from TStopwatch::Print()...
//    TThread::Printf("Total: %d, AvgPerEntry: %d",ttimeelapsed,ttimeelapsed/(double)completedEvents);
    Double_t  realt = totalTimer->RealTime();
    Double_t  cput  = totalTimer->CpuTime();
    Double_t avgt = totalTimer->RealTime()/(double)completedEvents;
 
    Int_t  hours = Int_t(realt / 3600);
    realt -= hours * 3600;
    Int_t  min   = Int_t(realt / 60);
    realt -= min * 60;
    Int_t  sec   = Int_t(realt);

    Int_t  avhours = Int_t(avgt / 3600);
    avgt -= avhours * 3600;
    Int_t  avmin   = Int_t(avgt / 60);
    avgt -= avmin * 60;
    Int_t  avsec   = Int_t(avgt);

 
    if (realt < 0) realt = 0;
    if (cput  < 0) cput  = 0;
    if (totalTimer->Counter() > 1) {
          TThread::Printf("Real time %d:%02d:%06.3f, CP time %.3f, %d slices", 
            hours, min, realt, cput, totalTimer->Counter());
    } 
    else {
          TThread::Printf("Real time %d:%02d:%06.3f, CP time %.3f", 
            hours, min, realt, cput);
          TThread::Printf("Average time per event: %d:%02d:%06.3f", 
            avhours, avmin, avgt);
    }


//    totalTimer->Print("m");

//   TThread::Printf("total events: %i",completedEvents);
    delete totalTimer;
    

    return;
}

/*
void TStopwatch::Print(Option_t *opt) const
 {
    Double_t  realt = const_cast<TStopwatch*>(this)->RealTime();
    Double_t  cput  = const_cast<TStopwatch*>(this)->CpuTime();
 
    Int_t  hours = Int_t(realt / 3600);
    realt -= hours * 3600;
    Int_t  min   = Int_t(realt / 60);
    realt -= min * 60;
    Int_t  sec   = Int_t(realt);
 
    if (realt < 0) realt = 0;
    if (cput  < 0) cput  = 0;
 
    if (opt && *opt == 'm') {
       if (Counter() > 1) {
          Printf("Real time %d:%02d:%06.3f, CP time %.3f, %d slices", hours, min, realt, cput, Counter());
       } else {
          Printf("Real time %d:%02d:%06.3f, CP time %.3f", hours, min, realt, cput);
       }
    } else if (opt && *opt == 'u') {
       if (Counter() > 1) {
          Printf("Real time %d:%02d:%09.6f, CP time %.3f, %d slices", hours, min, realt, cput, Counter());
       } else {
          Printf("Real time %d:%02d:%09.6f, CP time %.3f", hours, min, realt, cput);
       }
    } else {
       if (Counter() > 1) {
          Printf("Real time %d:%02d:%02d, CP time %.3f, %d slices", hours, min, sec, cput, Counter());
       } else {
          Printf("Real time %d:%02d:%02d, CP time %.3f", hours, min, sec, cput);
       }
    }
 }
*/



Int_t KScheduler::runThreads(){
    Int_t ret; 
    int i;
    std::cout << "KScheduler spawning threads..." << std::endl;
    std::cout << "KScheduler spawning readerThread..." << std::endl;
    ret = this->startReaderThread();
    assert(ret == 0);
    std::cout << "KScheduler spawning reaperThread..." << std::endl;
    ret = this->startReaperThread();
    assert(ret == 0);
    std::cout << "KScheduler spawning worker threads..." << std::endl;
    ret = this->startWorkerThreads();
    assert(ret == 0);

    // wait for threads to finish
    TThread::Join(fRDPtr->GetId(),NULL);
    delete fRDPtr;
    fRDPtr = 0;
    TThread::Join(fRPPtr->GetId(),NULL);
    delete fRPPtr;
    fRPPtr = 0;
    TThread* wPtr = 0;
    for(i=0;i<NTHREADS;i++){
        wPtr = workThreadArr[i];
        TThread::Join(wPtr->GetId(),NULL);
    }

    return 0;
}

// static getters and setters
TString KScheduler::getInputFilename(){
    return inputFilename;
}
void KScheduler::setInputFilename(TString name){
    inputFilename = name;
}
// static getters and setters
TString KScheduler::getOutputFilename(){
    return outputFilename;
}

void KScheduler::setOutputFilename(TString name){
    outputFilename = name;
}

void KScheduler::postCompletedEvent(){
    if(completedEvents % PRINTFREQ == 0){
        avgTimer->Stop();
        TThread::Printf("completed: %i events, last %i in time shown below:",
            completedEvents, PRINTFREQ);
        // uhoh gap here?
        avgTimer->Print("u");
        avgTimer->Start();
        TThread::Printf("\n");

    }
    completedEvents++;
};

// junk...
/*
 * _m_hitID_idx
 * _m_trghitID_idx
 * _event_header
 * _event
 * _spill_map
 * _triggerhit_vector
 * _hit_vector
 */

// converter from e1039 SQHitVector to SRawEvent for ktracker
SRawEvent* KScheduler::BuildSRawEvent(SQEvent* sqevent, SQHitVector* sqhitvector, SQHitVector* sqtrighitvector){
    SRawEvent* SRawEv = new SRawEvent();
    
    if(!UtilSRawEvent::SetEvent( SRawEv, sqevent)){
        // TODO what event override do I do?
        SRawEv->setEventInfo(0, 0, 0); 
    }

    // Doesn't seem to exist in data yet? SQSpillMap?
    /*
    if(_spill_map){
        UtilSRawEvent::SetSpill(SRawEv, _spill_map->get( SRawEv->getSpillID() ));
    }
    */

    // QIE not yet implemented
   
    // Trigger hits 
    // TODO keep hitid maps for updating later?
    UtilSRawEvent::SetTriggerHit(SRawEv, sqtrighitvector, NULL);
    UtilSRawEvent::SetHit(SRawEv, sqhitvector, NULL);

    return SRawEv;
}


// allocates memory for KJOBs - need to delete them in reaperThread
void* KScheduler::fReaderThread(void* readerArgPtr){

    int i;

    // wack... outdated instructions for ROOT?
    KScheduler* kschd = (KScheduler*) readerArgPtr;
    TString filename = kschd->getInputFilename();
    if(Verbose() > 0){
        TThread::Printf("Start of fReaderThread - ifile is: %s\n", filename.Data());
    }

    // to be filled from Tree
    // or from converter in e1039
    SRawEvent* localEventPtr=0;

    // in case we have E1039 data
    SQEvent* SQEvPtr = 0;
    SQHitVector* SQHitVecPtr = 0;
    SQHitVector* SQTrigHitVecPtr = 0;

    // rules of synch for ROOT obj are still not clear, so i just sync
    // everything.. Not sure if i need to use global mutex for aLL root obj or
    // local ones are good enough with the EnableThreadSafety feature of R6...
    // if its global i'm boned unless i set up a sleeping system for queue
    // filling... 
    //
    // GetEntries does make a new TObject... BUT IN ROOT6
    // ENABLETHREADSAFETYCLAIMSTHAT THIS IS ACCEPTABLEEEEEEEEE.... yikes


    // just lock everything for now... performace issues are better than race
    // cond.
    kschd->fReaderMutex->Lock();
    TFile* inputFile = new TFile(filename, "READ"); 
    TTree* dataTree = 0;
    if(use_e906_data){
        dataTree = (TTree*) inputFile->Get("save");
        dataTree->SetBranchAddress("rawEvent",&localEventPtr);
    }
    // e1039 data formatting...
    else{
        dataTree = (TTree*) inputFile->Get("T");
        dataTree->SetBranchAddress("DST.SQEvent",&SQEvPtr);
        dataTree->SetBranchAddress("DST.SQHitVector", &SQHitVecPtr);
        dataTree->SetBranchAddress("DST.SQTriggerHitVector", &SQTrigHitVecPtr);
    }
    int nEvents = dataTree->GetEntries();
    TThread::Printf("fReaderThread: starting read of %i events...\n",nEvents);
    kschd->fReaderMutex->UnLock();

    // KJob pointer to add to queue 
    KJob* newKJobPtr = NULL;

    bool copy = true;

    // TThread sched_yield? for IO block?
    for(i=0; i<nEvents; i++){
        dataTree->GetEntry(i);

        // convert SQHitVector to SRawEvent
        // TODO memory leak from new alloc in BuildSRawEvent?
        if(! use_e906_data){
            localEventPtr = KScheduler::BuildSRawEvent(SQEvPtr, SQHitVecPtr, SQTrigHitVecPtr);    
            copy = false;
        }

        // makes a copy of the SRawEvent
        // takes a mutex when allocating memory for the event
        newKJobPtr = new KJob(i,localEventPtr,kschd,copy);

        // need shared queue class
        // for now theres just one mutex....
        kschd->njqESem->Wait();
        kschd->newJobQueuePutMutex->Lock();
        kschd->newJobQueue.push(newKJobPtr);
        kschd->newJobQueuePutMutex->UnLock();
        kschd->njqFSem->Post();
    }

    TThread::Printf("fReaderThread: finished reading all events... dumping poison...");

    for(i=0;i<NTHREADS;i++){
        if(Verbose() > 0){
            TThread::Printf("poisoning...");
        }
        newKJobPtr = new KJob(true);
        kschd->njqESem->Wait();
        kschd->newJobQueuePutMutex->Lock();
        kschd->newJobQueue.push(newKJobPtr);
        kschd->newJobQueuePutMutex->UnLock();
        kschd->njqFSem->Post();
    }
    return 0;
}

// reaper thread
void* KScheduler::fReaperThread(void* reaperArg){

    KJob* tCompleteJobPtr = NULL;
    KScheduler* kschd = (KScheduler*) reaperArg;
    recoConsts* rc = recoConsts::instance();

    bool running = true;
    int poisonPills = 0;
    int nTracklets = 0;
    int sizeTrackArr = 0;

//    TThread::Printf("worker #%u got ptr:%p",dwArgPtr->threadId,wArgPtr);
    TThread::Printf("Starting fReaper thread\n");

    // output fields for saveTree->Fill
    TString outputFilename = kschd->getOutputFilename();
    SRawEvent* outputRawEventPtr = 0;
    SRecEvent* outputRecEventPtr = 0;
    TClonesArray* outputTracklets = new TClonesArray("Tracklet",1000);

    // setup the output files...
    TThread::Printf("opening output file\n");
    TFile* saveFile = new TFile(outputFilename.Data(), "recreate"); 
    TTree* saveTree = new TTree("save","save");
    saveTree->Branch("rawEvent", &outputRawEventPtr, 256000, 99);
    saveTree->Branch("recEvent", &outputRecEventPtr, 256000, 99);
    //saveTree->Branch("time", &time, "time/D");
    saveTree->Branch("outputTracklets", &outputTracklets, 256000, 99);
    saveTree->Branch("nTracklets", &nTracklets, "nTracklets/I");
    saveTree->Branch("sizeTrackArr", &sizeTrackArr, "nTracklets/I");

    // try to reap jobs forever
    while(running){

        // try to acquire a job from the queue...
        kschd->cjqFSem->Wait();
        kschd->cmpJobQueuePutMutex->Lock();
        tCompleteJobPtr = kschd->cmpJobQueue.front();
        kschd->cmpJobQueue.pop();
        kschd->cmpJobQueuePutMutex->UnLock();
        kschd->cjqESem->Post();

        // check for poison
        if(tCompleteJobPtr->isPoison){
            // cleanup job and die... 
            if(Verbose() > 0){
                TThread::Printf("ReaperThread got poison pill...");
            }
            delete tCompleteJobPtr;
            poisonPills++;
            if(poisonPills == NTHREADS){
                TThread::Printf("caught all pills...");
                break;
            }
            else{
                continue;
            }
        }

        // check for halted thread
        if(tCompleteJobPtr->p_JobStatus==HALTED){
           // job failed for some reason... 
           delete tCompleteJobPtr;
           continue;

        }
        
        if(Verbose() > 0){
            tCompleteJobPtr->jobMutex->Lock();
            TThread::Printf("fReaper gets jobId: %i, eventID: %i", 
                tCompleteJobPtr->jobId, tCompleteJobPtr->evData->getEventID());
            tCompleteJobPtr->jobMutex->UnLock();
        }

        // TODO CHECK JOB STATUS 
        // otherwise no atomic way to safely delete... because i'd be deleting
        // mutex too...
        // TODO PRAY .... is there a race conidition here? I'm 99% sure the
        // pipeline enforces enough serialism here tobe safe
        //
        // TODO UNLESS there's a 3rd party daemon that checsk events or jobs...
        // maybe when doing time scheduling? ... careful here...
        
        outputRawEventPtr = tCompleteJobPtr->evData;
        outputRecEventPtr = tCompleteJobPtr->recEvData;
        //outputTracklets = tCompleteJobPtr->tracklets;

        //outputTracklets = new TClonesArray(*(tCompleteJobPtr->tracklets));
        *outputTracklets = *(tCompleteJobPtr->tracklets);

        TClonesArray& ref_output = *outputTracklets;
        nTracklets = tCompleteJobPtr->nTracklets;
        sizeTrackArr = outputTracklets->GetEntries();
        assert(outputRawEventPtr);
        assert(outputRecEventPtr);
        assert(outputTracklets);
        if(Verbose() > 0){
            TThread::Printf("got tracklets: %i for disk for event: %i\n",outputTracklets->GetEntries(), 
                tCompleteJobPtr->evData->getEventID());
            TThread::Printf("outputTracklets pointer is: %p\n", outputTracklets);
        
            if(outputTracklets->GetEntries()>0){
                Tracklet* printer = (Tracklet*) ref_output[0];
                TThread::Printf("first tracklet for disk is%p\n",printer);
                if(printer)
                    printer->print();
            }
       }
        saveTree->Fill();
        if(saveTree->GetEntries() % SAVENUM == 0){
            TThread::Printf("fReaper saving another %i jobs", SAVENUM);
            saveTree->AutoSave("SaveSelf");
        }
        outputTracklets->Clear();
        // need to reuse these job objects because tracklets are large...
        delete tCompleteJobPtr;
        kschd->postCompletedEvent();
    }


    TThread::Printf("fReaper attempting to save tree");
    // save outputs
    saveTree->AutoSave("SaveSelf");
    saveFile->cd();
    saveTree->Write();
    saveFile->Close();
    
    // cleanup
    // oh no how do i clean these all up...
    // outputTracklets? do i need to delete shallow copy?
    return 0;
}

bool KScheduler::acceptEvent(SRawEvent* rawEvent)
{
//TODO GO BACK TO NO DEBUG - nwuerfel 12/9/20
//#ifdef _DEBUG_ON
    int eventID = rawEvent->getEventID();
    if(rawEvent->getNHitsInD0() > 350) return false;
    if(rawEvent->getNHitsInD1() > 350) return false;
    if(rawEvent->getNHitsInD2() > 170) return false;
    if(rawEvent->getNHitsInD3p() > 140) return false;
    if(rawEvent->getNHitsInD3m() > 140) return false;

   /* 
    if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[0]) > 15) return false;
    if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[1]) > 10) return false;
    if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[2]) > 10) return false;
    if(rawEvent->getNHitsInDetectors(detectorIDs_maskX[3]) > 10) return false;
    if(rawEvent->getNPropHitsAll() > 300) return false;

    if(rawEvent->getNRoadsPos() > 5) return false;
    if(rawEvent->getNRoadsNeg() > 5) return false;
    */

    return true;
}

// need to delete the memory for the worker thread for now
void* KScheduler::fWorkerThread(void* wArgPtr){

    //workerArg lwArg;
    workerArg* dwArgPtr = (workerArg*) wArgPtr;
    unsigned threadId = dwArgPtr->threadId;
    KScheduler* kschd = dwArgPtr->kschdPtr;
    recoConsts* rc = recoConsts::instance();
    KJob* tCompleteJobPtr = NULL;

    // worker tool pointers
    EventReducer* evReducer = NULL;
    KalmanFastTracking* kFastTracker = NULL;

    bool running = true;

    int recStatus;

    // more output stuff
    int nTracklets;
    int nEvents_dimuon=0;
//    std::list<Tracklet>& rec_tracklets = NULL;
//    TClonesArray& arr_tracklets = NULL;
//    TThread::Printf("worker #%u got ptr:%p",dwArgPtr->threadId,wArgPtr);
    TThread::Printf("Starting worker thread%u\n",threadId);

    // try to get jobs forever
    while(running){

        // try to acquire a job from the queue...
        kschd->njqFSem->Wait();
        kschd->newJobQueuePutMutex->Lock();
        tCompleteJobPtr = kschd->newJobQueue.front();
        assert(tCompleteJobPtr);
        kschd->newJobQueue.pop();
        kschd->newJobQueuePutMutex->UnLock();
        kschd->njqESem->Post();

        // TODO check for poison pill
        if(tCompleteJobPtr->isPoison){
            running = false;
            //put job in complete queue to kill next part of pipeline
            kschd->cjqESem->Wait();
            kschd->cmpJobQueuePutMutex->Lock();
            kschd->cmpJobQueue.push(tCompleteJobPtr);
            kschd->cmpJobQueuePutMutex->UnLock();
            kschd->cjqFSem->Post();
            break;
        }
       
        // check poison stream so no crash
        assert(tCompleteJobPtr->evData);

        // try to acquire a job from the queue...
        /// TODO stuff on the job..
        // semaphore enforces synchronization... (I hope...)
        if(Verbose() > 0){
            TThread::Printf("Worker %u gets jobId: %i, eventID: %i\n", 
                threadId, tCompleteJobPtr->jobId, tCompleteJobPtr->evData->getEventID());
        }

        // acquire an eventReducer...
        kschd->erqFSem->Wait();
        kschd->evRedQueuePutMutex->Lock();
        evReducer = kschd->eventReducerQueue.front();
        assert(evReducer);
        kschd->eventReducerQueue.pop();
        kschd->evRedQueuePutMutex->UnLock();
        kschd->erqESem->Post();

        if(Verbose() > 0){
            TThread::Printf("Worker %u gets evReducer: %p\n",threadId,evReducer);
        }

        // reduce the event...
        evReducer->reduceEvent(tCompleteJobPtr->evData);
        if(kschd->acceptEvent(tCompleteJobPtr->evData)){
            if(Verbose() > 0){
                TThread::Printf("Worker %u passed event: %i",
                    threadId,tCompleteJobPtr->evData->getEventID());
            }
        }
        else{
           TThread::Printf("Worker %u failed event: %i",
                threadId,tCompleteJobPtr->evData->getEventID());
        }

        // TODO update hit hinfo for the SQhitvector? needs a lot of
        // bookkeeping...

        // put eventReducer back in the queue...
        kschd->erqESem->Wait();
        kschd->evRedQueuePutMutex->Lock();
        kschd->eventReducerQueue.push(evReducer);
        kschd->evRedQueuePutMutex->UnLock();
        kschd->erqFSem->Post();

        // now the same for the fast tracker
        kschd->kftqFSem->Wait();
        kschd->kFTrkQueuePutMutex->Lock();
        kFastTracker = kschd->kFastTrkQueue.front();
        assert(kFastTracker);
        kschd->kFastTrkQueue.pop();
        kschd->kFTrkQueuePutMutex->UnLock();
        kschd->kftqESem->Post();
       

        // do something with the tracker
        if(Verbose() > 0){
            TThread::Printf("Worker %u gets kFastTracker: %p\n", threadId, kFastTracker);
        }
        // set the event
        recStatus = kFastTracker->setRawEvent(tCompleteJobPtr->evData); 
        if(recStatus != 0 && Verbose() > 0)
            TThread::Printf("kFastTrackRecStatus: %i",recStatus);
        tCompleteJobPtr->recEvData->setRecStatus(recStatus);
        tCompleteJobPtr->recEvData->setRawEvent(tCompleteJobPtr->evData);

        if(Verbose() > 0){
            TThread::Printf("Worker %u completed rawEventSet in KFT: %p\n",
                threadId, kFastTracker);
        }
        
        // TODO
        // Fill TClones Array

        TClonesArray& arr_tracklets = *(tCompleteJobPtr->tracklets);
        if(Verbose() > 0){
            TThread::Printf("job pointer for tracklets is%p\n",tCompleteJobPtr->tracklets);
            TThread::Printf("trackletsize:%i\n",tCompleteJobPtr->tracklets->GetEntries());
        }

        nTracklets = 0;
        Tracklet* printer = 0;

/*
        std::list<Tracklet>& rec_tracklets = kFastTracker->getFinalTracklets(); 
        for(std::list<Tracklet>::iterator iter = rec_tracklets.begin(); iter != rec_tracklets.end(); ++iter){

            iter->calcChisq();

            //TODO tracklets
            new(arr_tracklets[nTracklets++]) Tracklet(*iter);
            TThread::Printf("iterating through tracklet: %p",iter);
           
            tCompleteJobPtr->nTracklets++;
#ifndef _ENABLE_KF
            SRecTrack recTrack = iter->getSRecTrack();
            tCompleteJobPtr->recEvData->insertTrack(recTrack);
#endif
        }


#ifdef _ENABLE_KF
        // TODO add fitter stuff here
#endif        
*/

        /// Keep all trackets.  idx = 0 (D0/D1), 1 (D2), 2 (D3p/m), 3 (D2+3), 4 (D1+2+3)
        for (int idx = 0; idx <= 4; idx++) {
            std::list<Tracklet>& tracklets_temp = kFastTracker->getTrackletList(idx);
            for(std::list<Tracklet>::iterator iter = tracklets_temp.begin(); iter != tracklets_temp.end(); ++iter){
//                    TThread::Printf("evaluating tracklets");
                iter->calcChisq();
//                    iter->print();

                //TODO tracklets
                new(arr_tracklets[nTracklets++]) Tracklet(*iter);
                tCompleteJobPtr->nTracklets++;
            }
        }
        if(Verbose() > 0){
            TThread::Printf("arr_tracklet has %i entries for eventID: %i\n",arr_tracklets.GetEntries(), 
                tCompleteJobPtr->evData->getEventID());
            if(arr_tracklets.GetEntries() > 0){
                printer = (Tracklet*) arr_tracklets[0]; 
                TThread::Printf("first tracklet is:%p for eventID: %i\n",printer, tCompleteJobPtr->evData->getEventID());
                if(printer)
                    printer->print();
            }
       }


        // put tracker back in queue...
        kschd->kftqESem->Wait();
        kschd->kFTrkQueuePutMutex->Lock();
        kschd->kFastTrkQueue.push(kFastTracker);
        kschd->kFTrkQueuePutMutex->UnLock();
        kschd->kftqFSem->Post();
      
       
        
        //put job in complete queue
        kschd->cjqESem->Wait();
        kschd->cmpJobQueuePutMutex->Lock();
        kschd->cmpJobQueue.push(tCompleteJobPtr);
        kschd->cmpJobQueuePutMutex->UnLock();
        kschd->cjqFSem->Post();

    }

    // figure out right place to do this... malloced in startWorkerThread
    delete dwArgPtr;
    return 0;
}


// takes mem for thred
Int_t KScheduler::startReaderThread(){
    // check threadstatus
    std::cout << "Booting fReaderThread" << std::endl;
    if(!fRDPtr){
//        fRDPtr = new TThread("fReaderThread", (void(*)(void*)) &fReaderThread, (void*) this );
        fRDPtr = new TThread("fReaderThread", 
            (TThread::VoidRtnFunc_t) &fReaderThread, (void*) this );

        fRDPtr->Run();
        return 0;
    }
    return 1;
}
// frees mem for thred
Int_t KScheduler::stopReaderThread(){
    if(fRDPtr){
        TThread::Delete(fRDPtr);
        delete fRDPtr;
        fRDPtr = 0;
        return 0;
    }
    return 1;
}

// takes mem for thred
Int_t KScheduler::startReaperThread(){
    // check threadstatus
    std::cout << "Booting fReaperThread" << std::endl;
    if(!fRPPtr){
//        fRPPtr = new TThread("fReaperThread", (void(*)(void*)) &fReaperThread, (void*) this );
        fRPPtr = new TThread("fReaperThread", 
            (TThread::VoidRtnFunc_t) &fReaperThread, (void*) this );

        fRPPtr->Run();
        return 0;
    }
    return 1;
}
Int_t KScheduler::stopReaperThread(){
    if(fRPPtr){
        TThread::Delete(fRPPtr);
        delete fRPPtr;
        fRPPtr = 0;
        return 0;
    }
    return 1;
}

// takes mem for argptr, needs to be freed by the Kthread
// TODO FIX INPUT ARG TO THIS NO SLOT ID NEEDED
Int_t KScheduler::startWorkerThread(unsigned threadId){
    // check threadstatus
    std::cout << "Booting fWorkerThread:" << threadId << std::endl;
    std::string threadnm = "workerThread" + std::to_string(threadId);
    const char* formatTnm = threadnm.c_str();
    TThread* thisThread;
    workerArg* wArgPtr = new workerArg;
    wArgPtr->kschdPtr = this;
    wArgPtr->threadId = threadId;

    Printf("thread %u Gets Ptr:%p", threadId, wArgPtr);
    if(!workThreadArr[threadId]){
//        workThreadArr[threadId] = new TThread(formatTnm, (void(*)(void*)) &fWorkerThread, 
//            (void*) wArgPtr );
        workThreadArr[threadId] = new TThread(formatTnm, 
            (TThread::VoidRtnFunc_t) &fWorkerThread, (void*) wArgPtr);
        thisThread = workThreadArr[threadId];
        assert(thisThread != 0);
        thisThread->Run();
        return 0;
    }
    return 1;
}

// wrapper
Int_t KScheduler::startWorkerThreads(){
    Int_t ret;
    TThread* thisThread;
    for(unsigned i = 0; i < NTHREADS; i++){
        ret = startWorkerThread(i);
        // delay in print here is enough for the wArg to work out from run...
        // TODO NEED REAL SYNC FOR THAT (sig after set warg?)
        // THIS IS REALLY BAD TODO
        std::cout << "started thread:" << i << std::endl;
        assert(ret == 0);
    }
    return 0;
}

// TODO this is a little more confusing with worker ids
/*
Int_t KScheduler::stopWorkerThread(){
    if(workThreadArr[]){
        TThread::Delete(fRPPtr);
        delete fRPPtr;
        fRPPtr = 0;
        return 0;
    }
    return 1;

}
*/


