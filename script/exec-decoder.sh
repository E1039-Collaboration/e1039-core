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
N_EVT=0

OPTIND=1
while getopts ":v:osde:" OPT ; do
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
	e ) N_EVT=$OPTARG
            echo "  N of events: $N_EVT"
            ;;
    esac
done
shift $((OPTIND - 1))

if [ -z "$1" ] ; then
    echo "!!ERROR!!  The 1st argument must be run number or 'daemon'.  Abort."
    exit
fi

DIR_SCRIPT=$(dirname $(readlink -f $0))
if [ $DIR_SCRIPT = '/data2/e1039/script' ] ; then
    source /data2/e1039/this-e1039.sh
elif [ -z "$E1039_CORE" ] ; then
    echo '!!ERROR!!  "E1039_CORE" has not been set.  Abort.'
    exit
fi

umask 0002
export E1039_DECODER_MODE=$DECO_MODE

if [ $1 = 'daemon' ] ; then
    FN_LOG=/dev/shm/log-decoder-daemon.txt
    echo "Launch a daemon process."
    echo "  Log file = $FN_LOG"
    root.exe -b -q "$E1039_CORE/macros/online/Daemon4MainDaq.C" &>$FN_LOG &
else
    RUN=$1
    echo "Single-run decoding."
    root.exe -b -l <<-EOF
	.L $E1039_CORE/macros/online/Daemon4MainDaq.C
	StartDecoder($RUN, $N_EVT, $IS_ONLINE);
	EOF
fi
