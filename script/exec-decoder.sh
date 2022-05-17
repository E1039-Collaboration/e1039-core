#!/bin/bash
# Script to instantly execute the Main-DAQ decoder.
#
# Before executing this script, user has to set E1039_CORE (i.e. sourcing 
# 'this-e1039.sh') to specify the e1039-core library to be used.
#
# If this script is placed in "/data2/e1039/script", it is regarded as the 
# official decoding.  It sources the setup script (i.e. 
# /data2/e1039/this-e1039.sh) so that anyone (shifter) can execute this script
# without manual setting.

if [ $(hostname -s) != 'e1039prod1' ] ; then
    echo "!!ERROR!!  This script must be run on e1039prod1.  Abort."
    exit
fi

E1039_CORE_VERSION=default
IS_ONLINE=false
DECO_MODE=devel
LAUNCHER=no
N_EVT=0

OPTIND=1
while getopts ":v:osdle:" OPT ; do
    case $OPT in
        v ) E1039_CORE_VERSION=$OPTARG
            echo "  E1039_CORE version: $E1039_CORE_VERSION"
            ;;
        o ) IS_ONLINE=true
            echo "  Online mode: $IS_ONLINE"
            ;;
        s ) DECO_MODE=std
            echo "  Decoder mode: $DECO_MODE"
            ;;
        d ) DECO_MODE=devel
            echo "  Decoder mode: $DECO_MODE"
            ;;
        l ) LAUNCHER=yes
            echo "  Launcher mode: $LAUNCHER"
            ;;
	e ) N_EVT=$OPTARG
            echo "  N of events: $N_EVT"
            ;;
    esac
done
shift $((OPTIND - 1))

DIR_SCRIPT=$(dirname $(readlink -f $0))
if [ $DIR_SCRIPT = '/data2/e1039/script' ] ; then
    source /data2/e1039/this-e1039.sh
elif [ -z "$E1039_CORE" ] ; then
    echo '!!ERROR!!  "E1039_CORE" has not been set.  Abort.'
    exit
fi

umask 0002
export E1039_DECODER_MODE=$DECO_MODE

if [ $LAUNCHER = yes ] ; then
    FN_LOG=/dev/shm/log-decoder-daemon.txt
    echo "Launch a daemon process."
    echo "  Log file = $FN_LOG"
    root.exe -b -q "$E1039_CORE/macros/online/Daemon4MainDaq.C" &>$FN_LOG
else
    if [ -z "$1" ] ; then
	echo "The 1st argument must be a run number.  Abort."
	exit
    fi
    RUN=$1
    echo "Single-run decoding for run = $RUN."
    mkdir -p /dev/shm/decoder_maindaq
    FN_LOG=$(printf '/dev/shm/decoder_maindaq/log_%06d.txt' $RUN)
    if [ -e $FN_LOG ] ; then
	for (( II = 1 ;  ; II++ )) ; do
	    test ! -e $FN_LOG.$II && mv $FN_LOG $FN_LOG.$II && break
	done
    fi
    echo "  Log file = $FN_LOG"
    root.exe -b -l -q "$E1039_CORE/macros/online/Fun4MainDaq.C($RUN, $N_EVT, $IS_ONLINE)" &>$FN_LOG
    RET=$?
    SQMS_SEND=/data2/users/kenichi/local/SQMS/SQMS-sender.py
    if [ $RET -ne 0 -a $IS_ONLINE = true -a -e $SQMS_SEND ] ; then
	MSG="RUN: $RUN"$'\n'
	MSG+="RET: $RET"$'\n'
	MSG+="LOG: $FN_LOG"
	$SQMS_SEND -t 'exec-decoder.sh' -p 'E' -A 'Online Software Alarm,bhy7tf@virginia.edu' -m "$MSG"
    fi
fi
