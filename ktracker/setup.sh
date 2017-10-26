#Set locations
export KTRACKER_ROOT=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
export KTRACKER_LIB=$KTRACKER_ROOT/lib  #todo test for opt/dbg
export KTRACKER_INCLUDE=$KTRACKER_ROOT/include
export KTRACKER_BIN=$KTRACKER_ROOT/bin
export PYTHONPATH=$PYTHONPATH:$KTRACKER_ROOT/script/grid

#this package needs the geometry package.  check for it and try to find it
if [ ! -d "$GEOMETRY_ROOT" ]; then
  echo WARNING: You must setup the geometry package to run some kTracker programs.
  if [ -d "$SEAQUEST_INSTALL_ROOT" ]; then
    if [ -d "$SEAQUEST_INSTALL_ROOT/seaquest/geometry" ]; then
      source $SEAQUEST_INSTALL_ROOT/seaquest/geometry/setup.sh
      echo      OK, I found your geometry package at $GEOMETRY_ROOT
    else
      echo      The SeaQuest software distribution you are using does not have the geometry package installed
    fi
  else
    echo          You do not appear to be using the SeaQuest software distribution so I cannot find this package for you
    echo          Try 'source ../geometry/setup.sh'
  fi
fi

#this package needs the geometry package.  check for it and try to find it
if [ ! -d "$TRIGGER_ROOT" ]; then
  echo WARNING: You must setup the trigger package to run some kTracker programs.
  if [ -d "$SEAQUEST_INSTALL_ROOT" ]; then
    if [ -d "$SEAQUEST_INSTALL_ROOT/seaquest/trigger" ]; then
      source $SEAQUEST_INSTALL_ROOT/seaquest/trigger/setup.sh
      echo      OK, I found your trigger package at $TRIGGER_ROOT
    else
      echo      The SeaQuest software distribution you are using does not have the trigger package installed
    fi
  else
    echo          You do not appear to be using the SeaQuest software distribution so I cannot find this package for you
    echo          Try 'source ../trigger/setup.sh'
  fi
fi


#Set libs
#remove existing references to kTracker
if [ -f $SEAQUEST_SETUP_ROOT/setup_bash_utils.sh ]; then
  source $SEAQUEST_SETUP_ROOT/setup_bash_utils.sh
  export LD_LIBRARY_PATH=`minidropit $LD_LIBRARY_PATH seaquest/ktracker`
fi
export LD_LIBRARY_PATH=$KTRACKER_LIB:$LD_LIBRARY_PATH
