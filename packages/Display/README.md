# Event Display in `e1039-core`

This package is based on the TEve-based event display developed in sPHENIX.
Fundamentals are explained in [DocDB 5887](https://seaquest-docdb.fnal.gov/cgi-bin/private/ShowDocument?docid=5887)
and also in [this e1039-wiki page](https://github.com/E1039-Collaboration/e1039-wiki/wiki/How-to-run-event-display),
although they are getting obsolete.

As the event info is passed through the Fun4All framework,
various input types are possible.
Two types are supported as of January 2020.

## Standard Usage

There are two short-cut icons on the desktop of `e1039-monitor`.
One can just double-click them.


## Input from Full DST

The full DST file is created by the Main-DAQ decoder.
Read an event from it is rather straightforward in Fun4All.

A macro for this input type is `e1039-analysis/CODAChainDev/eve_disp_dst.C`.
A shell script, `/data2/e1039/monitor/exec-eve-disp.sh`, can be used
to set up the environment and run the macro on `e1039-monitor` etc.;
```
/data2/e1039/monitor/exec-eve-disp.sh
```
It asks you a run number to be displayed.

The full DST file is *not* readable when being written out.
Thus this input type does not work on a run being decoded (and acquired).


## Input from Special DST for Real-Time Display

This input type is intended to be used in the real-time display,
namely displaying a run being decoded (= acquired).

The decoder creates a special DST file, 
when properly configured in `e1039-core/online/macros/Fun4MainDaq.C`.
It contains only sampled events (typically a few events per spill)
and its tree header is updated every event (via TTree::AutoSave())
so that it is readable even when being written.
It is managed by `Fun4AllDstOutputManager` named `DSTOUT2`
and is saved under `/data2/e1039/onlmon/evt_disp`.
The event sampling is controlled by `EvtDispFilter`.

A macro for this input type is
`e1039-core/packages/Display/macro/EventDisp4MainDaqDst.C`.
A shell script in the same directory, `exec-eve-disp.sh`,
can be used to set up the environment and run the macro on `e1039-monitor` etc.


## To-Do Items

1. GUI for the 2nd input type is similar (copied) but separated from
   that for the 1st input type.  They should be unified.
1. The GUI functions are limited.  They should be expanded upon request.
1. The geometry configuration (like `G4_Target.C`) is locally defined.
   It should be shared with the other macros such as the simulation.
