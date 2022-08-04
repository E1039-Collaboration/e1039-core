/*
 * KScheduler.h
 *
 * also includes implementation of KJob
 * 
 * Author: Noah Wuerfel, nwuerfel@umich.edu
 * Created: 10-19-2020
 * AP AP AP AP
 *
 *
 */

#ifndef _KSCHEDULER_H_
#define _KSCHEDULER_H_

#include <GlobalConsts.h>
#include <geom_svc/GeomSvc.h>

#include <list>
#include <map>
#include <queue>
#include <new>

#include <TString.h>
#include <TRandom.h>
#include <TClonesArray.h>
#include <TStopwatch.h>
#include <TThread.h>
#include <TSemaphore.h>
#include <TMutex.h>
#include <TCondition.h>

#include <ktracker/EventReducer.h>
#include <ktracker/SRawEvent.h>
#include <ktracker/TriggerAnalyzer.h>
#include <ktracker/KalmanFastTracking.h>
#include <ktracker/UtilSRawEvent.h>

#include <interface_main/SQHit.h>
#include <interface_main/SQHit_v1.h>
#include <interface_main/SQHitMap_v1.h>
#include <interface_main/SQHitVector_v1.h>
#include <interface_main/SQEvent_v1.h>
#include <interface_main/SQRun_v1.h>
#include <interface_main/SQSpill_v1.h>
#include <interface_main/SQSpillMap_v1.h>
#include <interface_main/SQTrackVector_v1.h>

#define NTHREADS 16 // later make JobOpt
#define INPUT_PIPE_DEPTH 32 // play with iobuffer to load memory vs do computation...
#define OUTPUT_PIPE_DEPTH 32

#define NEVENT_REDUCERS NTHREADS// reducer factories
#define NKFAST_TRACKERS NTHREADS// tracking factories
#define PRINTFREQ 100 // num events per timeupdate TODO post fraction need update from fReaderThread
#define SAVENUM 100

enum KJobStatus {STARTED, RUNNING, HALTED, COMPLETE};
enum KJobQueueStatus {UNINITIALIZED, INITIALIZED, OPEN, FULL};

class KJob;
class SQEvent;
class SQHitMap;
class SQHitVector;

class KScheduler{

    public:
    KScheduler(TString inFile, TString outFile);
    ~KScheduler();

    void Init();

    static TString getInputFilename(); 
    static void setInputFilename(TString name); 
    static TString getOutputFilename();
    static void setOutputFilename(TString name);
    static void getTrackletsInStation(int stid);
    void postCompletedEvent();

    Int_t runThreads();

    static void Verbose(const int a) { verb = a; }
    static int  Verbose() { return verb; }

    void UseE906Data(const bool a) { use_e906_data = a; }
    bool UseE906Data() { return use_e906_data; }

    void UseTrackletReco(const bool a) { use_tracklet_reco = a; }
    bool UseTrackletReco() const { return use_tracklet_reco; }

    // output tracklets
    
    // tracklet outputs mutex
    TMutex* ktrkQueueMutex;

    // tracklets
    TSemaphore* ktrkqFSem;
    TSemaphore* ktrkqESem;

    // tracklet arrays...
    std::queue<TClonesArray*> kTrackArrQueue;

    private:

// methods


    // tmp TODO remove
    bool acceptEvent(SRawEvent* rawEvent);
    ///Configurations of tracklet finding
    //Hodo. IDs for masking, 4 means we have 4 hodo stations
    std::vector<int> detectorIDs_mask[4];
    std::vector<int> detectorIDs_maskX[4];
    std::vector<int> detectorIDs_maskY[4];
    std::list<int>   hitIDs_mask[4];              //hits in T/B, L/R are combined
    std::vector<int> detectorIDs_muidHodoAid[2];  //Aux-hodoscope masking for muon ID

    //SRawEvent* BuildSRawEvent(SQEvent* sqevent, SQHitVector* sqhitvector, SQHitVector* sqtrighitvector);

    // reader
    static void* fReaderThread(void* readerArg);
    Int_t startReaderThread();
    Int_t stopReaderThread();
    
    // reaper
    static void* fReaperThread(void* reaperArg);
    Int_t startReaperThread();
    Int_t stopReaperThread();


    // worker threads
    static void* fWorkerThread(void* wArg);
    Int_t startWorkerThread(unsigned threadId);
    Int_t startWorkerThreads();

    // e1039 data format converter
    static SRawEvent* BuildSRawEvent(SQEvent* sqevent, SQHitVector* sqhitvector, SQHitVector* sqtrighitvector);


// stuff
    static int verb;
    static bool use_e906_data;

    bool use_tracklet_reco;

    TriggerAnalyzer* p_triggerAna;

    //io thread
    static TString inputFilename;
    static TString outputFilename;

    // NOT PROTECTED ASSUMES ONE fREAPER
    static int completedEvents;
    static Double_t totalTimeElapsed;

    TStopwatch*  avgTimer;
    TStopwatch* totalTimer;

    //static TString outputFile;
    // reader
    TThread* fRDPtr;
    // reaper
    TThread* fRPPtr;

    // shooting in the dark with root multithreading...
    TMutex* wArgMutex;
    TMutex* fReaderMutex; 

    // reader 
    TMutex* newJobQueuePutMutex;
    TMutex* newJobQueueTakeMutex;

    // reducer
    TMutex* evRedQueuePutMutex;
    TMutex* evRedQueueTakeMutex;

    // tracker
    TMutex* kFTrkQueuePutMutex;
    TMutex* kFTrkQueueTakeMutex;

    // reaper
    TMutex* cmpJobQueuePutMutex;
    TMutex* cmpJobQueueTakeMutex;

    // input pipe
    std::queue<KJob*> newJobQueue; 

    // output pipe
    std::queue<KJob*> cmpJobQueue;


    // first stage sems
    TSemaphore* njqFSem;
    TSemaphore* njqESem;

    // secondstage pipielines
    TSemaphore* erqFSem;
    TSemaphore* erqESem;

    // finder sems
    TSemaphore* kftqFSem;
    TSemaphore* kftqESem;
    // out stage sems
    TSemaphore* cjqFSem;
    TSemaphore* cjqESem;

    // worker threads
    // TODO split into levels: eventReducer...
    std::array<TThread*, NTHREADS> workThreadArr;

    // event reducers.,.
    std::queue<EventReducer*> eventReducerQueue;
    // fast trackers...
    std::queue<KalmanFastTracking*> kFastTrkQueue;

    ClassDef(KScheduler,1)
};

class KJob{

public:
    KJob(int jobId, SRawEvent* evPtr, KScheduler* universe, bool copy);
    KJob(bool poisoned);
    ~KJob();

    static void Verbose(const int a) { verb = a; }
    static int  Verbose() { return verb; }

    //TODO needs a mutex     // TMUTEX 
    //KJobStatus getJobStatus();
    int jobId;
    KScheduler* universe;
    int nTracklets = 0;
    bool isPoison;
    // TODO probably dont need a jobmutex
    TMutex* jobMutex;
    TStopwatch* jobTimer;
    SRawEvent* evData;

    // tracklets
    TClonesArray* tracklets; 

    SRecEvent* recEvData;
    KJobStatus p_JobStatus;


private:
    static int verb;

    ClassDef(KJob,1)

};


#endif 
