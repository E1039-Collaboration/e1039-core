# Event Display in `e1039-core`

This package is based on the TEve-based event display developed in sPHENIX.
Fundamentals are explained in [DocDB 5887](https://seaquest-docdb.fnal.gov/cgi-bin/private/ShowDocument?docid=5887)
and also in [this e1039-wiki page](https://github.com/E1039-Collaboration/e1039-wiki/wiki/How-to-run-event-display),
although they are getting obsolete.
Technical details are explained in [DocDB 7302](https://seaquest-docdb.fnal.gov/cgi-bin/private/ShowDocument?docid=7302).

As the event info is passed through the Fun4All framework,
various input modes are possible.
Two modes are mainly supported as of February 2020, called "Online Mode" and "Offline Mode".

## Standard Usage at NM4

There are two short-cut icons on the desktop of `e1039-monitor`.
One can just double-click them.
They execute the shell script described below.

## Online Mode: Input from Special DST for Real-Time Display

This input mode is intended to be used in the real-time display,
namely displaying a run being decoded (= acquired).

The decoder creates a special DST file called "edDST" (= event-display DST), 
when properly configured in `e1039-core/online/macros/Fun4MainDaq.C`.
It contains only sampled events (typically a few events per spill)
and its tree header is updated every event (via TTree::AutoSave())
so that it is readable even when being written.
It is managed by `Fun4AllDstOutputManager` named `DSTOUT2`
and is saved under `/data2/e1039/online/evt_disp`.
The event sampling is controlled by `EvtDispFilter`.

A macro for this input mode is
`e1039-core/packages/Display/macro/EventDisp4MainDaqDst.C`.
To run this macro,
you had better use a shell script in the same directory, `exec-eve-disp.sh`,
namely execute `/path/to/exec-eve-displ.sh`.

## Offline Mode: Input from Full DST

The full DST file is created by the Main-DAQ decoder.
Reading an event from it is rather straightforward in Fun4All.
Note that this input mode does not work on a run being decoded (and acquired)
because the full DST file is not readable when being written out.

A macro for this input mode is again
`e1039-core/packages/Display/macro/EventDisp4MainDaqDst.C`.
To run this macro,
you had better execute `/path/to/exec-eve-displ.sh offline`.
It asks you a run number to be displayed.

## To-Do Items

1. The GUI functions are limited.  They should be expanded upon request.
1. The geometry configuration (like `G4_Target.C`) is locally defined.
   It should be shared with the other macros such as the simulation.
