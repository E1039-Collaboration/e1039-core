#!/bin/bash
RUN=${1:-3346}
SPILL=${2:-1}

if [ -z "$DIR_TOP" ] ; then
    echo "DIR_TOP is not defined.  Source 'setup.sh' first.  Abort."
    exit 1
fi

RUN6=$(printf '%06d' $RUN)
SPILL9=$(printf '%09d' $SPILL)
DIR_OUT=$DIR_ONLINE/srec/cpu/run_$RUN6

FN_IN=$DIR_ONLINE/sraw/run_$RUN6/run_${RUN6}_spill_${SPILL9}_sraw.root
FN_OUT=$DIR_OUT/run_${RUN6}_spill_${SPILL9}_srec.root

umask 0002
mkdir -p $DIR_OUT

root.exe -b -q "$DIR_TOP/RecoData.C($RUN, \"$FN_IN\", \"$FN_OUT\")" |& tee $DIR_OUT/run_${RUN6}_spill_${SPILL9}_srec.txt
RET=$?
exit $RET
