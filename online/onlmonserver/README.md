# Online Monitor (OnlMon) in e1039-core

The OnlMon system is based on the Fun4All framework for the event processing.

The OnlMon client (`OnlMonClient`) is inherited from `SubsysReco` to carry out the event analysis.
It is equipped with common functions needed for the OnlMon scheme.
All subsystem clients (like `OnlMonMainDaq`) are inherited from `OnlMonClient`.

The OnlMon server (`OnlMonServer`) is inherited from `Fun4AllServer` to input/process/output events.
It in addition accepts a network (TCP/IP) connection from the OnlMon viewer to provide histograms that each client holds.


## OnlMon Client

The functions of the OnlMon client are
1. Define a set of histograms to be monitored,
1. Fill the histograms per event when called by the OnlMon server, and
1. Draw the histograms when called by the OnlMon viewer.

Note that the OnlMon client runs in the two independent processes, namely the OnlMon server and the OnlMon viewer.
The histogram contents are shared via network communication.


## OnlMon Client of Subsystem Group

Each subsytem group is asked to develop and maintain its OnlMon client(s).
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


## Procedure for Developing OnlMonClient

It is explained in e1039-wiki.

