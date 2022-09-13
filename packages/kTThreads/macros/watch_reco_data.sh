#!/bin/bash
DIR_SCRIPT=$(dirname $(readlink -f $BASH_SOURCE))

TABLE_ANA='cpu_ana_status'

##
## Functions
##
function TableExists {
    local -r  FILE=$1
    local -r TABLE=$2
    local -r N_TBL=$(sqlite3 "file:$FILE?mode=ro" "select count(*) from sqlite_master where type='table' and name='$TABLE'")
    test $N_TBL -gt 0
}

function CreateTable {
    local -r  FILE=$1
    local -r TABLE=$2
    echo "Create the table: $TABLE."
    sqlite3 $FILE \
	"create table $TABLE ( 
                run_id         INTEGER, 
                spill_id       INTEGER, 
                utime_ana_cpu  INTEGER DEFAULT 0,
                status_ana_cpu INTEGER DEFAULT 0,
                UNIQUE(run_id, spill_id) )"
}

# As of 2022-08-04 the SQLite query becomes very slow if "order by ... desc" is used.
# The speed cannot be improved even with the "index" table created.
# Thus the descending sort is done with the "sort" command.
function FindAvailableData {
    local -r ANA_DB=$DIR_SCRIPT/ana_status.db
    local -r SEC_MAX=600

    local RET=
    for (( II = 0 ; II < 5 ; II++ )) ; do
	sqlite3 -separator ' ' "file:$STATUS_DB?mode=ro" \
	    "select run_id, spill_id 
               from $STATUS_TBL left join $TABLE_ANA using (run_id, spill_id) 
              where strftime('%s', 'now') - utime_reco_cpu < $SEC_MAX
                and status_reco_cpu = $DB_VAL_END
                and (status_ana_cpu is null or status_ana_cpu != $DB_VAL_END)" | sort -r -n -k 1,1 -k 2,2
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
	sqlite3 $STATUS_DB \
	    "insert or ignore into $TABLE_ANA (run_id, spill_id)
                            values ($RUN, $SPILL) ;
             update $TABLE_ANA set utime_ana_cpu = strftime('%s', 'now'),
                              status_ana_cpu = $VALUE
              where run_id = $RUN and spill_id = $SPILL"
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

if ! TableExists $STATUS_DB $TABLE_ANA ; then
    CreateTable $STATUS_DB $TABLE_ANA
    RET=$?
    if [ $? -ne 0 ] ; then
	echo "!!ERROR!!  CreateTable returned $RET.  Abort."
	exit 0
    fi
fi

while true ; do
    echo "Looping..."
    N_FILE=2 # Only a limited number of files is analyzed for now.
    FindAvailableData | while read RUN SPILL ; do
	echo "Run $RUN, Spill $SPILL, N $N_FILE"
	SetStatus $RUN $SPILL $DB_VAL_START

	$DIR_SCRIPT/exec_ana.sh $RUN $SPILL &>/dev/null
	RET=$?
	test $RET -ne 0 && echo "  Return = $RET"
	SetStatus $RUN $SPILL $DB_VAL_END

	test $(( --N_FILE )) -eq 0 && break
    done
    #echo "Wait for 30 s..."
    sleep 30
done


# create index idx_data_status on data_status (run_id desc, spill_id desc);
# PRAGMA index_list('data_status');
# PRAGMA index_info('idx_data_status');
# drop index idx_data_status;
