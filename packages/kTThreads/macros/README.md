# kTThreads/macros

Scripts and macros to launch the online multi-CPU reconstruction.

The e1039-core library (`e1039-core/packages/kTThreads`) does not work as is.
A modified version is being used for now, as seen in `setup.sh`.


## One-Time Execution

### Reconstruction

```
./watch_deco_data.sh
```

This script watches newly-decoded data (i.e. spill-by-spill SRawEvent files).
It executes `exec_reco.sh` per data file, which calls `RecoData.C`.

`exec_reco.sh` can be executely manually for test:
```
./exec_reco.sh 3346 1
```


## Analysis

```
./watch_reco_data.sh
```

This script watches newly-reconstructed data. 
It executes `exec_ana.sh` per data file, which calls `AnaData.C`.


## Daemonized Execution

Keep both `watch_deco_data.sh` and `watch_reco_data.sh` running in background.
`/home/Tracking/kenichi/script/keep-daemon-up.sh` is a script for it.

