# Online Monitor (OnlMon) in e1039-core

The structure of the OnlMon system in terms of code and process is illustrated in [DocDB 6106](https://seaquest-docdb.fnal.gov/cgi-bin/private/ShowDocument?docid=6106).

## OnlMon Client

The functions of the OnlMon client are
1. Define a set of histograms to be monitored,
1. Fill the histograms per event when called by the OnlMon server, and
1. Draw the histograms when called by the OnlMon viewer.

Note that the OnlMon client runs in the two independent processes, namely the OnlMon server and the OnlMon viewer.
The histogram contents are shared via network communication.

## Development of OnlMon-Client Class by Subsystem Group

Each subsytem group is asked to develop and maintain its OnlMon-client class.
It should not be much compilcated because a templete is available.

The six functions listed below should be modified consistently with one another.
Details are not available yet but will be described in the Doxygen document.

 | Function             | Role |
 |----------------------|------|
 | InitOnlMon()         | Initialize all variables that don't depend on the run info. |
 | InitRunOnlMon()	| Initialize all variables that depend on the run info.       |
 | ProcessEventOnlMon()	| Process one given event to fill histograms.                 |
 | EndOnlMon()		| Carry out any calculations (like efficiency) after all events are processed. |
 | FindAllMonHist()	| Associate histogram objects found by name with histogram variables.  |
 | DrawMonitor()        | Design canvases, draw histograms, and decide the run-quality status. |

## Code Management

Below is the recommended procedure for updating the OnlMon code.

1. Make a new branch on GitHub and modify it.
1. Run a test, as instructed below.
1. Make a pull request with a short description of code modification and test result.
1. Ask the online-software coordinator to review and accept the pull request.
1. Let the online-software coordinator compile the updated code in the official directory.
1. Confirm that the updated code works fine on one or two incoming runs.

## Test Procedure

When a client class is updated, it has to be tested before merged into the master branch.
The test procedure varies with the location of update.

### When only the Drawing Style of Histograms is Updated

It is rather simple because the official process of the OnlMon server can be used.
You can start the OnlMon viewer in your working directory to draw the histograms.
```
root /path/to/e1039-core/online/macros/OnlMon4MainDaq.C
```

### When the Number or Filling Condition of Histograms is Updated

It is complicated because you have to run both the OnlMon server and the OnlMon viewer using your updated code.
It must not interfere with the official OnlMon process.

For now our system is not capable of doing it.
Thus please confirm that the updated code is compiled fine, skip the test for now, and make a pull request.
The online-software coordinator will run the test.
