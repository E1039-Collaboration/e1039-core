16,18c16
< #include <GlobalConsts.h>
< #include <geom_svc/GeomSvc.h>
< 
---
> #include "GlobalConsts.h"
27,36c25,37
< #include <TStopwatch.h>
< #include <TThread.h>
< #include <TSemaphore.h>
< #include <TMutex.h>
< #include <TCondition.h>
< 
< #include <ktracker/EventReducer.h>
< #include <ktracker/SRawEvent.h>
< #include <ktracker/TriggerAnalyzer.h>
< #include <ktracker/KalmanFastTracking.h>
---
> 
> #include "TStopwatch.h"
> #include "EventReducer.h"
> #include "GeomSvc.h"
> #include "SRawEvent.h"
> #include "TriggerAnalyzer.h"
> #include "JobOptsSvc.h"
> #include "TThread.h"
> #include "TSemaphore.h"
> #include "TMutex.h"
> #include "TCondition.h"
> #include "assert.h"
> #include "KalmanFastTracking.h"
39,45c40,46
< #define NTHREADS 4 // later make JobOpt
< #define INPUT_PIPE_DEPTH 4 // play with iobuffer to load memory vs do computation...
< #define OUTPUT_PIPE_DEPTH 4
< 
< #define NEVENT_REDUCERS 4 // reducer factories
< #define NKFAST_TRACKERS 4 // tracking factories
< #define PRINTFREQ 10 // num events per timeupdate TODO post fraction need update from fReaderThread
---
> #define NTHREADS 32 // later make JobOpt
> #define INPUT_PIPE_DEPTH 32 // play with iobuffer to load memory vs do computation...
> #define OUTPUT_PIPE_DEPTH 32
> 
> #define NEVENT_REDUCERS NTHREADS// reducer factories
> #define NKFAST_TRACKERS NTHREADS// tracking factories
> #define PRINTFREQ 100 // num events per timeupdate TODO post fraction need update from fReaderThread
51,76c52
< class KJob{
< 
< public:
<     KJob(int jobId, SRawEvent* evPtr);
<     KJob(bool poisoned);
<     ~KJob();
< 
<     //TODO needs a mutex     // TMUTEX 
<     //KJobStatus getJobStatus();
<     int jobId;
<     int nTracklets = 0;
<     bool isPoison;
<     // TODO probably dont need a jobmutex
<     TMutex* jobMutex;
<     TStopwatch* jobTimer;
<     SRawEvent* evData;
<     SRecEvent* recEvData;
<     KJobStatus p_JobStatus;
< 
< private:
< 
<     ClassDef(KJob,1)
< 
< };
< 
< 
---
> class KJob;
87a64
>     static void getTrackletsInStation(int stid);
91a69,80
>     // output tracklets
>     
>     // tracklet outputs mutex
>     TMutex* ktrkQueueMutex;
> 
>     // tracklets
>     TSemaphore* ktrkqFSem;
>     TSemaphore* ktrkqESem;
> 
>     // tracklet arrays...
>     std::queue<TClonesArray*> kTrackArrQueue;
> 
124c113,116
<     // stuff
---
> 
> 
> // stuff
> 
125a118
>     JobOptsSvc* p_jobOptsSvc;
127a121,122
>     static int trackletStationId;
> 
170a166
> 
182d177
< 
197a193,223
> 
> class KJob{
> 
> public:
>     KJob(int jobId, SRawEvent* evPtr, KScheduler* universe);
>     KJob(bool poisoned);
>     ~KJob();
> 
>     //TODO needs a mutex     // TMUTEX 
>     //KJobStatus getJobStatus();
>     int jobId;
>     KScheduler* universe;
>     int nTracklets = 0;
>     bool isPoison;
>     // TODO probably dont need a jobmutex
>     TMutex* jobMutex;
>     TStopwatch* jobTimer;
>     SRawEvent* evData;
> 
>     // tracklets
>     TClonesArray* tracklets; 
> 
>     SRecEvent* recEvData;
>     KJobStatus p_JobStatus;
> 
> private:
> 
>     ClassDef(KJob,1)
> 
> };
> 
