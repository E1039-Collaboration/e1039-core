# Installation/Compilation guide for kTracker

Since version R007, kTracker uses cmake as the build tool. A CMakelist.txt file is provided with complete set of adjustable parameters. This is intended to eliminate the needs to modify the MODE_SWITCH.h file.

***

### Prerequisites

kTracker relies on following packages to run correctly:
- MySQL: `mysql-config` should be in the `$PATH`
- ROOT v5.34: `root-config` should be in the `$PATH`. ROOT v6 should work, but has not been tested
- Geant v4.9.6.p04: `geant4-config` should be in the `$PATH`. Geant4.10 does not work with this version of kTracker
- BOOST: if not installed in system locations (/usr or /usr/local), environmental variable `BOOST_ROOT` needs to be set

CMake will search for these packages and quit if any of them is not found.

### Step-by-step build guide

- Checkout the source code: `ssh://p-seaquest-ktracker@cdcvs.fnal.gov/cvs/projects/seaquest-ktracker`
- Go to the checkout directory and set up the environment: `source setup.sh`
- Create the build location: `mkdir build`
- Go to the build directory and generate makefile: `cmake ..`. In this step cmake will check the dependences and create Makefiles for each .cxx file located in exe directory. If user place their own program in this directory a Makefile will be automatically generated for it.
- Compile: `make`. All the targets will be compiled as an executable binary in kTracker directory. A shared library libkTracker.so will also be generated in kTracker/lib to keep backward compatibility.

### Available parameters

Following parameters could be used in CMake:
- DEBUG1: equivalent to old DEBUG_ON in MODE_SWITCH.h, default OFF
- DEBUG2: equivalent to old DEBUG_ON_LEVEL_2 in MODE_SWITCH.h, default OFF
- KAMG: turn KMAG ON and OFF, default ON
- KFENABLE: enable Kalman filter based fitting, default ON
- ALIGNMENTMODE: add the alignment tools to the build, also disables a lot of features in kTracker, for alignment program only, default OFF

Eventually all the switches in MODE_SWITCH.h will be moved to CMakeList.txt file

### Remaining tweaks in GlobalConsts.h (before called MODE_SWITCH.h)

As for now, there are still a couple of things that are hard coded in MODE_SWITCH.h, they are:
- FMAGSTR and KMAGSTR: the magnetic field strength multiplier;
- X_VTX and Y_VTX: the center of beam position;
- BEAM_SPOT_X and BEAM_SPOT_Y: the size, or sigma of the beam spot assuming beam has 2-D gaussian shape;
- MERGE_THRES: threshold for merging station-0 and station-1 tracks
