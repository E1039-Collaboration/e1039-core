#!/bin/bash
DIR_SCRIPT=$(dirname $(readlink -f $BASH_SOURCE))

##
## Functions
##
function FindAvailableData {
    local -r SEC_MAX=600

    local RET=
    for (( II = 0 ; II < 5 ; II++ )) ; do
	sqlite3 -separator ' ' "file:$STATUS_DB?mode=ro" \
	    "select run_id, spill_id   from $STATUS_TBL
              where strftime('%s', 'now') - utime_deco < $SEC_MAX
                and status_deco = $DB_VAL_END
                and status_reco_cpu = $DB_VAL_UNDEF" | sort -r -n -k 1,1 -k 2,2
        #order by run_id desc, spill_id desc"
	RET=$?
	test $RET -eq 0 && break
	echo "  II=$II"
	sleep 1
    done
    test $RET -ne 0 && echo "FindAvailableData failed with ret = $RET."
    return $RET
}

function SetStatus {
    local -r   RUN=$1
    local -r SPILL=$2
    local -r VALUE=$3

    local RET=
    for (( II = 0 ; II < 5 ; II++ )) ; do
	sqlite3 $STATUS_DB "update $STATUS_TBL set utime_reco_cpu = strftime('%s', 'now'), status_reco_cpu = $VALUE where run_id = $RUN and spill_id = $SPILL"
	
	RET=$?
	test $RET -eq 0 && break
	echo "  II=$II"
	sleep 1
    done
    test $RET -ne 0 && echo "SetStatus failed with ret = $RET."
    return $RET
}

##
## Main
##
source $DIR_SCRIPT/setup.sh
umask 0002

while true ; do
    echo "Looping..."
    N_FILE=5 # Only a limited number of files is analyzed for now.
    FindAvailableData | while read RUN SPILL ; do
	echo "Run $RUN, Spill $SPILL, N $N_FILE"
	SetStatus $RUN $SPILL $DB_VAL_START

	$DIR_SCRIPT/exec_reco.sh $RUN $SPILL &>/dev/null
	RET=$?
	test $RET -ne 0 && echo "  Return = $RET"

	SetStatus $RUN $SPILL $DB_VAL_END
	test $(( --N_FILE )) -eq 0 && break
    done
    #echo "Wait for 30 s..."
    sleep 30
done
